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

#include <storage/storage.h>
#include <string.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>

#include "tools/defs.h"
#include "tools/fkit/meds.h"
#include "tools/fkit/history.h"

#include <takeyourmeds_icons.h>


typedef struct {
    View*       view;
    bool        use_text;
    char        title[32];
    char        text[32];
    const Icon* icon;
    uint8_t     icon_x;
    uint8_t     icon_y;
    bool        closeable;
    bool        exit_on_back;
} SplashScreen;

typedef struct {
    ViewDispatcher* dispatcher;
    VariableItemList* menu_edit_med;
    Submenu* menu_manage;

    SplashScreen splash_screens[SPLASHES_COUNT + 1];

    char new_med_name[MED_NAME_MAX_SIZE];

    bool     splash_is_active;
    uint32_t splash_start_ticks;
    uint8_t  splash_target_view_after;
} AppContext;


static uint32_t tick = 0;

static uint32_t current_view_id;

static Medicine meds[MEDS_CAPACITY];
static Medicine* target_med;
static size_t meds_count = 0;

static void rebuild_menu_edit_med(VariableItemList* menu);
static void menu_manage_callback(void* context, uint32_t index);


//////////////////
///  WRAPPERS  ///
//////////////////

static void switch_to_view(AppContext* app, uint8_t view_id) {
    if(!app || !app->dispatcher) return;
    view_dispatcher_switch_to_view(app->dispatcher, view_id);
    app->splash_is_active = false;
    current_view_id = view_id;
}

////////////////
///  GLOBAL  ///
////////////////

static bool app_navigation_callback(void* context) {
    UNUSED(context);
    return false;
}

static void tick_callback(void* context) {
    AppContext* app = context;

    tick++;

    if (app && app->splash_is_active && current_view_id <= SPLASHES_COUNT) {
        if (app->splash_screens[current_view_id].closeable && tick >= app->splash_start_ticks + SPLASH_DURATION_TICKS) {
            switch_to_view(app, app->splash_target_view_after);
        }
    }
}

////////////////////////
///  SPLASH SCREENS  ///
////////////////////////

static void switch_to_splash(AppContext* app, uint8_t splash_id, uint8_t next_view_id) {
    switch_to_view(app, splash_id);

    app->splash_is_active = true;
    app->splash_start_ticks = tick;
    app->splash_target_view_after = next_view_id;
}

static bool splash_input_callback(InputEvent* event, void* context) {
    AppContext* app = context;
    if(!app || !event) return false;
    ViewDispatcher* dispatcher = app->dispatcher;

    if (event->type == InputTypePress) {
        if (event->key == InputKeyBack && app->splash_screens[current_view_id].exit_on_back) {
            view_dispatcher_stop(dispatcher);
        } else {
            switch_to_view(app, app->splash_target_view_after);
        }
        return true;
    }

    return false;
}

static void splash_draw_callback(Canvas* canvas, void* context) {
    AppContext* app = context;
    if(!app || current_view_id > SPLASHES_COUNT) return;
    SplashScreen* splash = &app->splash_screens[current_view_id];

    canvas_clear(canvas);
    canvas_draw_icon(
        canvas,
        splash->icon_x, splash->icon_y,
        splash->icon
    );
    if (splash->use_text) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, SPLASH_TEXT_X, SPLASH_TITLE_Y, splash->title);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, SPLASH_TEXT_X, SPLASH_TEXT_Y, splash->text);
    }
}

///////////////////
///  MAIN MENU  ///
///////////////////

static void menu_main_callback(void* context, uint32_t index) {
    AppContext* app = context;
    if(!app) return;

    FURI_LOG_D(APP_LOG_NAME, "Main menu item %lu selected", (unsigned long)index);

    switch(index) {
    case MENUITEM_MAIN_CHECKIN:
        FURI_LOG_W(APP_LOG_NAME, "MENUITEM_MAIN_CHECKIN is not implemented");
        break;

    case MENUITEM_MAIN_STATS:
        FURI_LOG_W(APP_LOG_NAME, "MENUITEM_MAIN_STATS is not implemented");
        break;

    case MENUITEM_MAIN_MANAGE:
        switch_to_view(app, VIEWID_MENU_MANAGE);
        break;

    case MENUITEM_MAIN_SETTINGS:
        FURI_LOG_W(APP_LOG_NAME, "MENUITEM_MAIN_SETTINGS is not implemented");
        break;

    case MENUITEM_MAIN_ABOUT:
        switch_to_view(app, VIEWID_TEXT_ABOUT);
        break;

    case MENUITEM_MAIN_EXIT:
        view_dispatcher_stop(app->dispatcher);
        break;
    }
}

//////////////////////
///  MANAGER MENU  ///
//////////////////////

static void rebuild_menu_manage(Submenu* menu, AppContext* app) {
    submenu_reset(menu);

    submenu_set_header(menu, "Manage meds");

    for (uint8_t idx = 0; idx < MEDS_CAPACITY; idx++) {
        if (idx < meds_count) {
            submenu_add_item(menu, meds[idx].name, idx, menu_manage_callback, app);
        } else {
            submenu_add_item(menu, "(Empty slot)", idx, menu_manage_callback, app);
        }
    }
    submenu_add_item(menu, "Back", MEDS_CAPACITY, menu_manage_callback, app);
}

static void menu_manage_callback(void* context, uint32_t index) {
    AppContext* app = context;
    if(!app) return;

    if (index >= MEDS_CAPACITY) {
        switch_to_view(app, VIEWID_MENU_MAIN);
        return;
    }

    target_med = &meds[index];

    if (index >= meds_count) {
        memset(app->new_med_name, 0, sizeof(app->new_med_name));
        switch_to_view(app, VIEWID_INPUT_NEW_MED_NAME);
        return;
    }

    rebuild_menu_edit_med(app->menu_edit_med);
    switch_to_view(app, VIEWID_EDIT_MED);
}

static uint32_t manager_previous_callback(void* context) {
    UNUSED(context);
    return VIEWID_MENU_MAIN;
}

////////////////////////////
///  NEW MED NAME INPUT  ///
////////////////////////////

static void input_new_med_name_callback(void* context) {
    AppContext* app = context;
    if(!app || !target_med) return;

    if(meds_count >= MEDS_CAPACITY || app->new_med_name[0] == '\0') {
        switch_to_splash(app, VIEWID_SPLASH_ERROR, VIEWID_MENU_MANAGE);
        return;
    }

    // Always leave room for the terminating NUL byte.
    strncpy(target_med->name, app->new_med_name, sizeof(target_med->name) - 1);
    target_med->name[sizeof(target_med->name) - 1] = '\0';
    target_med->takes = 1;

    const size_t new_count = meds_count + 1;
    if(dump_meds(meds, new_count, APP_DATA_PATH(PATH_FILE_MEDS))) {
        meds_count = new_count;
        rebuild_menu_manage(app->menu_manage, app);
        switch_to_splash(app, VIEWID_SPLASH_NEW_MED_SUCCESS, VIEWID_MENU_MANAGE);
        return;
    }

    // Persistence failed: keep the in-memory slot available for another attempt.
    memset(target_med, 0, sizeof(*target_med));
    switch_to_splash(app, VIEWID_SPLASH_ERROR, VIEWID_MENU_MANAGE);
}

///////////////////////
///  EDIT MED MENU  ///
///////////////////////

static void set_takes_for_selected_med(VariableItem* item) {
    if(!item || !target_med) return;
    uint8_t index = variable_item_get_current_value_index(item);
    const char* values[] = {"1", "2", "3", "4", "5", "6"};
    if(index >= (sizeof(values) / sizeof(values[0]))) return;
    target_med->takes = index + 1;
    variable_item_set_current_value_text(item, values[index]);
}

static void rebuild_menu_edit_med(VariableItemList* menu) {
    variable_item_list_reset(menu);

    VariableItem* last = variable_item_list_add(
        menu,
        "Takes per day",
        6,
        set_takes_for_selected_med, NULL
    );
    if(!last) {
        FURI_LOG_E(APP_LOG_NAME, "Failed to create medicine setting item");
        return;
    }
    variable_item_set_current_value_index(last, 0);
    variable_item_set_current_value_text(last, "1");
}

static uint32_t menu_edit_med_previous_callback(void* context) {
    UNUSED(context);
    return VIEWID_MENU_MANAGE;
}

//////////////////////
///  ABOUT SCREEN  ///
//////////////////////

static uint32_t about_previous_callback(void* context) {
    UNUSED(context);
    return VIEWID_MENU_MAIN;
}

///////////////////
///  APP SETUP  ///
///////////////////

int32_t takeyourmeds_app(void* p) {
    UNUSED(p);

    path_history = APP_DATA_PATH(PATH_FILE_HISTORY);

    AppContext app = {0};

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* dispatcher = view_dispatcher_alloc();
    app.dispatcher = dispatcher;
    view_dispatcher_set_event_callback_context(dispatcher, &app);
    view_dispatcher_set_navigation_event_callback(dispatcher, app_navigation_callback);
    view_dispatcher_attach_to_gui(dispatcher, gui, ViewDispatcherTypeFullscreen);

    Submenu* menu_main = submenu_alloc();
    submenu_set_header(menu_main, APP_DISPLAY_NAME);
    submenu_add_item(menu_main, "Check in",    MENUITEM_MAIN_CHECKIN,  menu_main_callback, &app);
    submenu_add_item(menu_main, "My stats",    MENUITEM_MAIN_STATS,    menu_main_callback, &app);
    submenu_add_item(menu_main, "Manage meds", MENUITEM_MAIN_MANAGE,   menu_main_callback, &app);
    submenu_add_item(menu_main, "Settings",    MENUITEM_MAIN_SETTINGS, menu_main_callback, &app);
    submenu_add_item(menu_main, "About",       MENUITEM_MAIN_ABOUT,    menu_main_callback, &app);
    submenu_add_item(menu_main, "Exit",        MENUITEM_MAIN_EXIT,     menu_main_callback, &app);
    
    Submenu* menu_manage = submenu_alloc();
    app.menu_manage = menu_manage;
    view_set_previous_callback(submenu_get_view(menu_manage), manager_previous_callback);

    TextInput* input_new_med_name = text_input_alloc();
    text_input_set_header_text(input_new_med_name, "Medicine name");
   
    VariableItemList* menu_edit_med = variable_item_list_alloc();
    view_set_previous_callback(variable_item_list_get_view(menu_edit_med), menu_edit_med_previous_callback);
    app.menu_edit_med = menu_edit_med;

    TextBox* about = text_box_alloc();
    text_box_set_text(
        about,
        APP_DISPLAY_NAME
        " (v"
        APP_VERSION
        ")\n"
        "\n"
        "Keep forgetting to take your meds? Gamify the process!\n"
        "This app can motivate you to do this regularly by tracking your daily streak (and rewarding you with achievements).\n"
        "\n"
        "Created by Anna Romashina <3\n"
        "github.com/R1senDev/fap_"
        APP_REG_NAME
        "\n"
        "\n"
        "\"Care for your health more!\""
    );
    view_set_previous_callback(text_box_get_view(about), about_previous_callback);
    
    app.splash_screens[VIEWID_SPLASH_AUTHOR].view         = view_alloc();
    app.splash_screens[VIEWID_SPLASH_AUTHOR].use_text     = false;
    app.splash_screens[VIEWID_SPLASH_AUTHOR].icon         = &I_Author;
    app.splash_screens[VIEWID_SPLASH_AUTHOR].icon_x       = 0;
    app.splash_screens[VIEWID_SPLASH_AUTHOR].icon_y       = 0;
    app.splash_screens[VIEWID_SPLASH_AUTHOR].closeable    = 0;
    app.splash_screens[VIEWID_SPLASH_AUTHOR].exit_on_back = true;

    app.splash_screens[VIEWID_SPLASH_ERROR].view         = view_alloc();
    app.splash_screens[VIEWID_SPLASH_ERROR].use_text     = true;
    strcpy(app.splash_screens[VIEWID_SPLASH_ERROR].title, "Error :(");
    strcpy(app.splash_screens[VIEWID_SPLASH_ERROR].text, "Trying again may help");
    app.splash_screens[VIEWID_SPLASH_ERROR].icon         = &I_FDolphin_Error;
    app.splash_screens[VIEWID_SPLASH_ERROR].icon_x       = 0;
    app.splash_screens[VIEWID_SPLASH_ERROR].icon_y       = 0;
    app.splash_screens[VIEWID_SPLASH_ERROR].closeable    = 0;
    app.splash_screens[VIEWID_SPLASH_ERROR].exit_on_back = false;

    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].view         = view_alloc();
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].use_text     = true;
    strcpy(app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].title, "Added successfully!");
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].text[0]      = '\0';
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].icon         = &I_FDolphin_Success;
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].icon_x       = 0;
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].icon_y       = 0;
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].closeable    = 0;
    app.splash_screens[VIEWID_SPLASH_NEW_MED_SUCCESS].exit_on_back = false;

    rebuild_menu_manage(menu_manage, &app);

    text_input_set_result_callback(
        input_new_med_name,
        input_new_med_name_callback, &app,
        app.new_med_name, sizeof(app.new_med_name),
        true
    );

    for (uint8_t view_id = 1; view_id <= SPLASHES_COUNT; view_id++) {
        View* target = app.splash_screens[view_id].view;
        view_set_context(target, &app);
        view_set_draw_callback(target, splash_draw_callback);
        view_set_input_callback(target, splash_input_callback);
        view_dispatcher_add_view(dispatcher, view_id, target);
    }
    
    view_dispatcher_add_view(dispatcher, VIEWID_MENU_MAIN,          submenu_get_view(menu_main));
    view_dispatcher_add_view(dispatcher, VIEWID_TEXT_ABOUT,         text_box_get_view(about));
    view_dispatcher_add_view(dispatcher, VIEWID_MENU_MANAGE,        submenu_get_view(menu_manage));
    view_dispatcher_add_view(dispatcher, VIEWID_INPUT_NEW_MED_NAME, text_input_get_view(input_new_med_name));
    view_dispatcher_add_view(dispatcher, VIEWID_EDIT_MED,           variable_item_list_get_view(menu_edit_med));

    switch_to_splash(&app, VIEWID_SPLASH_AUTHOR, VIEWID_MENU_MAIN);

    view_dispatcher_set_tick_event_callback(
        dispatcher,
        tick_callback,
        furi_ms_to_ticks(MS_PER_TICK)
    );

    view_dispatcher_run(dispatcher);

    for(uint8_t view_id = 1; view_id <= SPLASHES_COUNT; view_id++) {
        view_dispatcher_remove_view(dispatcher, view_id);
        view_free(app.splash_screens[view_id].view);
    }
    view_dispatcher_remove_view(dispatcher, VIEWID_MENU_MAIN);
    view_dispatcher_remove_view(dispatcher, VIEWID_TEXT_ABOUT);
    view_dispatcher_remove_view(dispatcher, VIEWID_MENU_MANAGE);
    view_dispatcher_remove_view(dispatcher, VIEWID_INPUT_NEW_MED_NAME);
    view_dispatcher_remove_view(dispatcher, VIEWID_EDIT_MED);

    submenu_free(menu_main);
    text_box_free(about);
    submenu_free(menu_manage);
    text_input_free(input_new_med_name);
    variable_item_list_free(menu_edit_med);

    target_med = NULL;
    view_dispatcher_free(dispatcher);
    furi_record_close(RECORD_GUI);

    return 0;
}