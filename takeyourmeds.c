// Patience is the key.

#include <furi.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/variable_item_list.h>

#include "tools/defs.h"
#include "tools/fkit/meds.h"
#include "tools/fkit/history.h"

#include <takeyourmeds_icons.h>


///////////////////
///  APP SETUP  ///
///////////////////

int32_t takeyourmeds_app(void* p) {
    UNUSED(p);
    
    return 0;
}