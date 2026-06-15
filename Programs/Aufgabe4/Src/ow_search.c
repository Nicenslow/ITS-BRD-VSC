/**
 ******************************************************************************
 * @file    ow_search.c
 * @brief   Binary-Tree-Search mit Discrepancy-Tracking (AN187).
 ******************************************************************************
 */

#include "ow_search.h"

#include "1wire.h"
#include "crc8.h"

/** @brief Globaler Search-State gemaess AN187 */
uint8_t ROM_NO[8];
int     LastDiscrepancy;
int     LastFamilyDiscrepancy;
bool    LastDeviceFlag;

static uint8_t s_search_crc8;

static uint8_t ow_search_docrc8(uint8_t value) {
    s_search_crc8 = crc8_update(s_search_crc8, value);
    return s_search_crc8;
}

static bool ow_search_run(void) {
    int id_bit_number;
    int last_zero;
    int rom_byte_number;
    bool search_result;
    int id_bit;
    int cmp_id_bit;
    uint8_t rom_byte_mask;
    uint8_t search_direction;

    id_bit_number   = 1;
    last_zero       = 0;
    rom_byte_number = 0;
    rom_byte_mask   = 1U;
    search_result   = false;
    s_search_crc8   = 0U;

    if (LastDeviceFlag) {
        return false;
    }

    if (!ow_reset()) {
        LastDiscrepancy        = 0;
        LastDeviceFlag         = false;
        LastFamilyDiscrepancy  = 0;
        return false;
    }

    ow_write_byte(OW_CMD_SEARCH_ROM);

    do {
        id_bit     = (int)ow_read_bit();
        cmp_id_bit = (int)ow_read_bit();

        if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
        }

        if (id_bit != cmp_id_bit) {
            search_direction = (uint8_t)id_bit;
        } else {
            if (id_bit_number < LastDiscrepancy) {
                search_direction =
                    ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0U) ? 1U : 0U;
            } else {
                search_direction = (id_bit_number == LastDiscrepancy) ? 1U : 0U;
            }

            if (search_direction == 0U) {
                last_zero = id_bit_number;

                if (last_zero < 9) {
                    LastFamilyDiscrepancy = last_zero;
                }
            }
        }

        if (search_direction == 1U) {
            ROM_NO[rom_byte_number] |= rom_byte_mask;
        } else {
            ROM_NO[rom_byte_number] &= (uint8_t)~rom_byte_mask;
        }

        ow_write_bit(search_direction);

        id_bit_number++;

        rom_byte_mask <<= 1U;
        if (rom_byte_mask == 0U) {
            (void)ow_search_docrc8(ROM_NO[rom_byte_number]);
            rom_byte_number++;
            rom_byte_mask = 1U;
        }
    } while (rom_byte_number < 8);

    if (!((id_bit_number < 65) || (s_search_crc8 != 0U))) {
        LastDiscrepancy = last_zero;

        if (LastDiscrepancy == 0) {
            LastDeviceFlag = true;
        }

        search_result = true;
    }

    if (!search_result || (ROM_NO[0] == 0U)) {
        LastDiscrepancy       = 0;
        LastDeviceFlag        = false;
        LastFamilyDiscrepancy = 0;
        search_result         = false;
    }

    return search_result;
}

/**
 * @brief  Sucht den ersten Sensor auf dem Bus.
 */
bool ow_search_first(uint8_t rom[8]) {
    LastDiscrepancy       = 0;
    LastDeviceFlag        = false;
    LastFamilyDiscrepancy = 0;

    if (!ow_search_run()) {
        return false;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        rom[i] = ROM_NO[i];
    }

    return true;
}

/**
 * @brief  Sucht den naechsten Sensor auf dem Bus.
 */
bool ow_search_next(uint8_t rom[8]) {
    if (!ow_search_run()) {
        return false;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        rom[i] = ROM_NO[i];
    }

    return true;
}
