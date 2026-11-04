//////////////////////////////////////////
//  _      __              _            //
// | | /| / /__ ________  (_)__  ___ _  //
// | |/ |/ / _ `/ __/ _ \/ / _ \/ _ `/  //
// |__/|__/\_,_/_/ /_//_/_/_//_/\_, /   //
//                             /___/    //
//                                      //
//         YOU HAVE BEEN WARNED.        //
//                                      //
//               +------+               //
//               |  OK  |               //
//               +------+               //
//////////////////////////////////////////


#include <furi.h>
#include <furi_hal_rtc.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>

#include <takeyourmeds_icons.h>


#define VIEWID_SPLASH   0 // Done
#define VIEWID_MAINMENU 1 // Done
#define VIEWID_CHECKIN  2
#define VIEWID_STATS    3
#define VIEWID_MANAGE   4 // WIP
#define VIEWID_SETTINGS 5
#define VIEWID_ABOUT    6 // Done
#define VIEWID_EDIT_MED 7
#define VIEWID_NEW_MED  8

#define MENUITEM_MAIN_CHECKIN  0
#define MENUITEM_MAIN_STATS    1
#define MENUITEM_MAIN_MANAGE   2 // WIP
#define MENUITEM_MAIN_SETTINGS 3
#define MENUITEM_MAIN_ABOUT    4 // Works
#define MENUITEM_MAIN_EXIT     5 // Works

#define MEDS_CAPACITY 16


static uint32_t tick = 0;

static uint32_t current_view_id;
static uint8_t  target_med;

typedef struct {
    char name[17];
    uint8_t takes;
} Medicine;
static Medicine meds[16];
size_t meds_count = 0;

//////////////////
///  WRAPPERS  ///
//////////////////

static void switch_to_view(ViewDispatcher* dispatcher, uint32_t view_id) {
    view_dispatcher_switch_to_view(dispatcher, view_id);
    current_view_id = view_id;
}

////////////////
///  GLOBAL  ///
////////////////

static bool app_navigation_callback(void* context) {
    UNUSED(context);
    return false;
}

///////////////////////
///  SPLASH SCREEN  ///
///////////////////////

static void splash_tick_callback(void* context) {
    ViewDispatcher* dispatcher = context;
    tick++;

    if (current_view_id == VIEWID_SPLASH && tick >= 30) {
        switch_to_view(dispatcher, VIEWID_MAINMENU);
    }
}

static bool splash_input_callback(InputEvent* event, void* context) {
    ViewDispatcher* dispatcher = context;

    if (event->type == InputTypePress) {
        if (event->key == InputKeyBack) {
            view_dispatcher_stop(dispatcher);
        } else {
            switch_to_view(dispatcher, VIEWID_MAINMENU);
        }
        return true;
    }

    return false;
}

static void splash_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);

    canvas_clear(canvas);
    canvas_draw_icon(canvas, 0, 0, &I_Splash);
}

///////////////////
///  MAIN MENU  ///
///////////////////

static void menu_main_callback(void* context, uint32_t index) {
    ViewDispatcher* dispatcher = context;

    FURI_LOG_D("TakeYourMeds", "Main menu item %lu selected", (unsigned long)index);

    switch(index) {
    case MENUITEM_MAIN_CHECKIN:
        FURI_LOG_W("TakeYourMeds", "MENUITEM_MAIN_CHECKIN is not implemented");
        break;

    case MENUITEM_MAIN_STATS:
        FURI_LOG_W("TakeYourMeds", "MENUITEM_MAIN_STATS is not implemented");
        break;

    case MENUITEM_MAIN_MANAGE:
        switch_to_view(dispatcher, VIEWID_MANAGE);
        break;

    case MENUITEM_MAIN_SETTINGS:
        FURI_LOG_W("TakeYourMeds", "MENUITEM_MAIN_SETTINGS is not implemented");
        break;

    case MENUITEM_MAIN_ABOUT:
        switch_to_view(dispatcher, VIEWID_ABOUT);
        break;

    case MENUITEM_MAIN_EXIT:
        view_dispatcher_stop(dispatcher);
        break;
    }
}

//////////////////////
///  MANAGER MENU  ///
//////////////////////

static void menu_manage_callback(void* context, uint32_t index) {
    ViewDispatcher* dispatcher = context;

    if (index >= MEDS_CAPACITY) {
        switch_to_view(dispatcher, VIEWID_MAINMENU);
        return;
    }

    if (index >= meds_count) {
        switch_to_view(dispatcher, VIEWID_NEW_MED);
        return;
    }

    target_med = index;
    switch_to_view(dispatcher, VIEWID_EDIT_MED);
}

static uint32_t manager_previous_callback(void* context) {
    UNUSED(context);
    return VIEWID_MAINMENU;
}

//////////////////////
///  ABOUT SCREEN  ///
//////////////////////

static uint32_t about_previous_callback(void* context) {
    UNUSED(context);
    return VIEWID_MAINMENU;
}

///////////////////
///  APP SETUP  ///
///////////////////

int32_t takeyourmeds_app(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(dispatcher, dispatcher);
    view_dispatcher_set_navigation_event_callback(dispatcher, app_navigation_callback);
    view_dispatcher_attach_to_gui(dispatcher, gui, ViewDispatcherTypeFullscreen);

    View* splash = view_alloc();
    view_set_context(splash, dispatcher);
    view_set_draw_callback(splash, splash_draw_callback);
    view_set_input_callback(splash, splash_input_callback);

    Submenu* menu_main = submenu_alloc();
    submenu_set_header(menu_main, "Take your Meds");
    submenu_add_item(menu_main, "Check in",    MENUITEM_MAIN_CHECKIN,  menu_main_callback, dispatcher);
    submenu_add_item(menu_main, "My stats",    MENUITEM_MAIN_STATS,    menu_main_callback, dispatcher);
    submenu_add_item(menu_main, "Manage meds", MENUITEM_MAIN_MANAGE,   menu_main_callback, dispatcher);
    submenu_add_item(menu_main, "Settings",    MENUITEM_MAIN_SETTINGS, menu_main_callback, dispatcher);
    submenu_add_item(menu_main, "About",       MENUITEM_MAIN_ABOUT,    menu_main_callback, dispatcher);
    submenu_add_item(menu_main, "Exit",        MENUITEM_MAIN_EXIT,     menu_main_callback, dispatcher);

    TextBox* about = text_box_alloc();
    text_box_set_text(
        about,
        "Take your Meds (v0.1)\n"
        "\n"
        "Keep forgetting to take your meds? Gamify it!\n"
        "This app can motivate you to do this regularly by tracking your daily streak (and rewarding you with achievements).\n"
        "\n"
        "Created by Anna Romashina <3\n"
        "github.com/R1senDev/takeyourmeds\n"
        "\n"
        "\"Care for your health more!\""
    );
    view_set_previous_callback(text_box_get_view(about), about_previous_callback);

    Submenu* menu_manage = submenu_alloc();
    submenu_set_header(menu_manage, "Manage meds");
    for (uint8_t idx = 0; idx < MEDS_CAPACITY; idx++) {
        if (idx < meds_count) {
            submenu_add_item(menu_manage, meds[idx].name, idx, menu_manage_callback, dispatcher);
        } else {
            submenu_add_item(menu_manage, "(Empty slot)", idx, menu_manage_callback, dispatcher);
        }
    }
    submenu_add_item(menu_manage, "Back", MEDS_CAPACITY, menu_manage_callback, dispatcher);
    view_set_previous_callback(submenu_get_view(menu_manage), manager_previous_callback);
    
    view_dispatcher_add_view(dispatcher, VIEWID_SPLASH,   splash);
    view_dispatcher_add_view(dispatcher, VIEWID_MAINMENU, submenu_get_view(menu_main));
    view_dispatcher_add_view(dispatcher, VIEWID_ABOUT,    text_box_get_view(about));
    view_dispatcher_add_view(dispatcher, VIEWID_MANAGE,   submenu_get_view(menu_manage));

    switch_to_view(dispatcher, VIEWID_SPLASH);

    view_dispatcher_set_tick_event_callback(
        dispatcher,
        splash_tick_callback,
        furi_ms_to_ticks(100)
    );

    view_dispatcher_run(dispatcher);

    view_dispatcher_remove_view(dispatcher, VIEWID_SPLASH);
    view_dispatcher_remove_view(dispatcher, VIEWID_MAINMENU);
    view_dispatcher_remove_view(dispatcher, VIEWID_ABOUT);
    view_dispatcher_remove_view(dispatcher, VIEWID_MANAGE);

    view_free(splash);
    submenu_free(menu_main);
    text_box_free(about);
    submenu_free(menu_manage);

    view_dispatcher_free(dispatcher);
    furi_record_close(RECORD_GUI);

    return 0;
}