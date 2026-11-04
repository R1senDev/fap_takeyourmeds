#define APP_DISPLAY_NAME "Take your Meds"
#define APP_LOG_NAME     "TakeYourMeds"
#define APP_REG_NAME     "takeyourmeds"
#define APP_VERSION      "0.1"

#define PATH_FILE_MEDS    "meds.bin"
#define PATH_FILE_HISTORY "history.bin"

#define MS_PER_TICK 100

#define SPLASH_DURATION_TICKS 30
#define SPLASH_TITLE_Y 70
#define SPLASH_TEXT_Y  60
#define SPLASH_TEXT_X  64

#define VIEWID_SPLASH_AUTHOR           1 // Splash screen                        //              Done
#define VIEWID_SPLASH_ERROR            2 // Error popup                          // Queued
#define VIEWID_SPLASH_NEW_MED_SUCCESS  3 // Success popup                        // Queued
#define VIEWID_MENU_MAIN               4 // Main menu                            //              Done
#define VIEWID_MENU_CHECKIN            5 // Check-in menu                        // Queued
#define VIEWID_VIEW_STATS              6 // Stats screen                         // Queued
#define VIEWID_MENU_MANAGE             7 // Meds management menu                 //         WIP
#define VIEWID_MENU_SETTINGS           8 // Settings menu                        // Queued
#define VIEWID_TEXT_ABOUT              9 // About info screen                    //              Done
#define VIEWID_INPUT_NEW_MED_NAME     10 // New med name input                   //         WIP
#define VIEWID_VIEW_BUSY              11 // Heavy IO operation in progress popup // Queued
#define VIEWID_EDIT_MED               12 // Med editing menu                     //              REQUIRES TESTING

#define VIEWID_MAX     12
#define SPLASHES_COUNT  3

#define MENUITEM_MAIN_CHECKIN  0
#define MENUITEM_MAIN_STATS    1
#define MENUITEM_MAIN_MANAGE   2
#define MENUITEM_MAIN_SETTINGS 3
#define MENUITEM_MAIN_ABOUT    4
#define MENUITEM_MAIN_EXIT     5

#define MEDS_CAPACITY     16
#define MED_NAME_MAX_SIZE 17 // Including ZT