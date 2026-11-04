#include <furi.h>
#include <furi_hal_rtc.h>

#include "day.h"


uint32_t today(void) {
    uint32_t ts = furi_hal_rtc_get_timestamp();
    return ts / 86400;
}