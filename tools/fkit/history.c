#include <furi.h>
#include <storage/storage.h>

#include "../day.h"

#include "history.h"


const char* path_history;

enum {
    HISTORY_RECORD_SIZE = sizeof(uint32_t) + sizeof(uint8_t) + MEDS_CAPACITY * (sizeof(((MedTakeRecord*)0)->name) + sizeof(uint8_t) + sizeof(uint8_t))
};

static bool history_open(File* file, int mode, int open_mode) {
    return storage_file_open(file, path_history, mode, open_mode);
}

static bool history_position(File* file, uint32_t offset) {
    if (offset > INT32_MAX / HISTORY_RECORD_SIZE) return false;
    return storage_file_seek(file, (int32_t)(offset * HISTORY_RECORD_SIZE), true);
}

static bool history_read_record(File* file, MedTakeDay* day) {
    if (storage_file_read(file, &day->day, sizeof(day->day)) != sizeof(day->day)) return false;
    if (storage_file_read(file, &day->count, sizeof(day->count)) != sizeof(day->count)) return false;
    if (day->count > MEDS_CAPACITY) return false;

    memset(day->meds, 0, sizeof(day->meds));

    for (uint8_t i = 0; i < day->count; i++) {
        MedTakeRecord* medicine = &day->meds[i];

        if (storage_file_read(file, medicine->name, sizeof(medicine->name)) != sizeof(medicine->name)) {
            return false;
        }

        if (!memchr(medicine->name, '\0', sizeof(medicine->name))) return false;

        if (storage_file_read(file, &medicine->required_takes, sizeof(medicine->required_takes)) != sizeof(medicine->required_takes) || storage_file_read(file, &medicine->actual_takes, sizeof(medicine->actual_takes)) != sizeof(medicine->actual_takes)) {
            return false;
        }
    }

    return true;
}

static bool history_write_record(File* file, const MedTakeDay* day) {
    if (day->count > MEDS_CAPACITY) return false;
    if (storage_file_write(file, &day->day, sizeof(day->day)) != sizeof(day->day) || storage_file_write(file, &day->count, sizeof(day->count)) != sizeof(day->count)) {
        return false;
    }

    for (uint8_t i = 0; i < day->count; i++) {
        const MedTakeRecord* medicine = &day->meds[i];

        if (!memchr(medicine->name, '\0', sizeof(medicine->name))) return false;

        if (storage_file_write(file, medicine->name, sizeof(medicine->name)) != sizeof(medicine->name) || storage_file_write(file, &medicine->required_takes, sizeof(medicine->required_takes)) != sizeof(medicine->required_takes) || storage_file_write(file, &medicine->actual_takes, sizeof(medicine->actual_takes)) != sizeof(medicine->actual_takes)) {
            return false;
        }
    }

    return true;
}

bool history_get_day(uint32_t offset, MedTakeDay* day) {
    if (!day || offset > today()) return false;

    memset(day, 0, sizeof(*day));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool success = history_open(file, FSAM_READ, FSOM_OPEN_EXISTING) && history_position(file, offset) && history_read_record(file, day);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if (!success || day->day != today() - offset) {
        memset(day, 0, sizeof(*day));
        return false;
    }
    return true;
}

bool history_set_day(uint32_t offset, const MedTakeDay* day) {
    if (!day || offset > today() || day->day != today() - offset || day->count > MEDS_CAPACITY) {
        return false;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool success = history_open(file, FSAM_WRITE, FSOM_OPEN_ALWAYS) && history_position(file, offset) && history_write_record(file, day);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool history_set_actual_takes(uint32_t offset, const char* name, uint8_t actual_takes) {
    if (!name) return false;

    MedTakeDay day;
    if (!history_get_day(offset, &day)) return false;

    for (uint8_t i = 0; i < day.count; i++) {
        if (strncmp(day.meds[i].name, name, sizeof(day.meds[i].name)) == 0) {
            day.meds[i].actual_takes = actual_takes;
            return history_set_day(offset, &day);
        }
    }
    return false;
}

bool history_is_day_complete(uint32_t offset) {
    MedTakeDay day;
    if (!history_get_day(offset, &day)) return false;

    for (uint8_t i = 0; i < day.count; i++) {
        if (day.meds[i].actual_takes < day.meds[i].required_takes) return false;
    }

    return true;
}

uint32_t history_current_streak(void) {
    uint32_t streak = 0;
    while (history_is_day_complete(streak)) streak++;
    return streak;
}

uint32_t history_best_streak(void) {
    uint32_t best = 0;
    uint32_t streak = 0;
    uint32_t current_day = today();

    for (uint32_t offset = 0; offset <= current_day; offset++) {
        if (history_is_day_complete(offset)) {
            streak++;
            if (streak > best) best = streak;
        } else {
            streak = 0;
        }
    }
    return best;
}
