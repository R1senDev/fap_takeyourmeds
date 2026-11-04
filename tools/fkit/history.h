#pragma once

#include "../defs.h"


typedef struct {
    char name[MED_NAME_MAX_SIZE];
    uint8_t required_takes;
    uint8_t actual_takes;
} MedTakeRecord;

typedef struct {
    uint32_t day;
    uint8_t count;
    MedTakeRecord meds[MEDS_CAPACITY];
} MedTakeDay;


extern const char* path_history;

// Offset is measured in days and couts back from today (0 = today, 1 - yesterday, etc.)
bool history_get_day(uint32_t offset, MedTakeDay* day);
bool history_set_day(uint32_t offset, const MedTakeDay* day);

bool history_set_actual_takes(
    uint32_t offset,
    const char* name,
    uint8_t actual_takes
);

bool history_is_day_complete(uint32_t offset);

uint32_t history_current_streak(void);
uint32_t history_best_streak(void);