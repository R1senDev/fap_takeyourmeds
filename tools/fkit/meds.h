#pragma once

#include "../defs.h"


typedef struct {
    char name[MED_NAME_MAX_SIZE];
    uint8_t takes;
} Medicine;


bool reset_meds_file(const char* path);

bool load_meds(Medicine* meds, size_t capacity, size_t* count, const char* path);

bool dump_meds(const Medicine* meds, size_t count, const char* path);