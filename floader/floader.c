#include <furi.h>
#include <storage/storage.h>

#include "floader.h"


bool reset_file(const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool success = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if (success) {
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool load_meds(Medicine* meds, size_t capacity, size_t* count, const char* path) {
    if (!meds || !count || !path) return false;

    FURI_LOG_D(APP_LOG_NAME, "Begin meds file read");

    *count = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if (!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    bool success = true;

    uint8_t byte;
    uint16_t read;
    while (*count < capacity) {
        Medicine medicine = {0};
        size_t name_length = 0;

        while (true) {
            read = storage_file_read(file, &byte, 1);

            if (read != 1) {
                if (name_length == 0) {
                    goto done;
                }

                success = false;
                goto done;
            }

            if (byte == '\0') break;

            if (name_length >= sizeof(medicine.name) - 1) {
                success = false;
                goto done;
            }

            medicine.name[name_length++] = (char)byte;
        }

        if (name_length < 2) {
            success = false;
            goto done;
        }

        read = storage_file_read(file, &medicine.takes, sizeof(medicine.takes));

        if (read != sizeof(medicine.takes)) {
            success = false;
            goto done;
        }

        medicine.name[name_length] = '\0';
        meds[(*count)++] = medicine;
    }

    read = storage_file_read(file, &byte, 1);
    if (read != 0) success = false;

    done:

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if (!success) {
        FURI_LOG_E(APP_LOG_NAME, "Bad file. Resetting it");
        reset_file(path);
    }

    return success;
}

bool dump_meds(const Medicine* meds, size_t count, const char* path) {
    if ((!meds && count > 0) || !path) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if (!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    bool success = true;

    for (size_t i = 0; i < count; i++) {
        size_t name_length = strnlen(meds[i].name, sizeof(meds[i].name));

        if (name_length < 2 || name_length > 16) {
            success = false;
            break;
        }

        if (storage_file_write(file, meds[i].name, name_length + 1) != name_length + 1) {
            success = false;
            break;
        }

        if (storage_file_write(file, &meds[i].takes, sizeof(meds[i].takes)) != sizeof(meds[i].takes)) {
            success = false;
            break;
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}