/**
 ******************************************************************************
 * @file    ow_search.c
 * @brief   1-Wire Search-Algorithmus (Maxim AN187) – Teilaufgabe 3.
 *
 * === Problem ===
 *   Am Bus haengen mehrere Sensoren mit unterschiedlichen 64-Bit-ROMs.
 *   Read ROM (0x33) funktioniert nur bei genau einem Geraet.
 *
 * === Loesung ===
 *   Search ROM (0xF0) durchlaeuft einen Binaerbaum aller moeglichen ROMs.
 *   Pro Bit-Position liest der Master zwei Bits (echt + Komplement):
 *     - Beide gleich (0/1 oder 1/0): alle Slaves senden dasselbe -> Pfad fest
 *     - Beide 0 (Widerspruch): Slaves unterscheiden sich -> Master waehlt 0 oder 1
 *   LastDiscrepancy merkt sich, wo beim naechsten Lauf die andere Richtung probiert wird.
 *
 * === API ===
 *   ow_search_first() – neue Suche, erster Sensor
 *   ow_search_next()  – weiterer Sensor (State bleibt erhalten)
 ******************************************************************************
 */

#include "ow_search.h"

#include "1wire.h"
#include "crc8.h"

/** Globaler Search-State – zwischen ow_search_first() und ow_search_next() noetig */
uint8_t ROM_NO[8];              /* zuletzt gefundenes ROM (8 Bytes) */
int     LastDiscrepancy;        /* Bit-Position des letzten Widerspruchs */
int     LastFamilyDiscrepancy;  /* Widerspruch im Family-Code-Bereich */
bool    LastDeviceFlag;         /* true = alle Geraete gefunden */

static uint8_t s_search_crc8;

/** CRC waehrend der Suche mitfuehren (muss am Ende 0 ergeben) */
static uint8_t ow_search_docrc8(uint8_t value) {
    s_search_crc8 = crc8_update(s_search_crc8, value);
    return s_search_crc8;
}

/**
 * @brief  Kern des Search-Algorithmus – ein Durchlauf findet genau ein ROM.
 *
 * 64 Iterationen (je ein Bit des ROM):
 *   - 2 Bits lesen (id_bit, cmp_id_bit)
 *   - Suchrichtung waehlen (bei Widerspruch: LastDiscrepancy)
 *   - 1 Bit schreiben (Master bestaetigt gewaehlte Richtung)
 *   - ROM_NO[] und CRC mitfuehren
 */
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

    ow_write_byte(OW_CMD_SEARCH_ROM);   /* Befehl 0xF0 – Suche starten */

    do {
        /* Jede Slave sendet ihr Bit UND das invertierte Bit (Collision Detection) */
        id_bit     = (int)ow_read_bit();
        cmp_id_bit = (int)ow_read_bit();

        /* 1/1 = kein Geraet antwortet mehr -> Suche abbrechen */
        if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
        }

        if (id_bit != cmp_id_bit) {
            /* Kein Widerspruch: alle Slaves senden dasselbe Bit */
            search_direction = (uint8_t)id_bit;
        } else {
            /* Widerspruch (0/0): mehrere Slaves mit unterschiedlichen Bits */
            if (id_bit_number < LastDiscrepancy) {
                /* Bereits gewaehlter Pfad beibehalten */
                search_direction =
                    ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0U) ? 1U : 0U;
            } else {
                /* An der Diskrepanz-Stelle: diesmal die andere Richtung (1) waehlen */
                search_direction = (id_bit_number == LastDiscrepancy) ? 1U : 0U;
            }

            if (search_direction == 0U) {
                last_zero = id_bit_number;   /* merken fuer naechsten Search-Lauf */

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

        ow_write_bit(search_direction);   /* Master bestaetigt gewaehltes Bit */

        id_bit_number++;

        rom_byte_mask <<= 1U;
        if (rom_byte_mask == 0U) {
            (void)ow_search_docrc8(ROM_NO[rom_byte_number]);
            rom_byte_number++;
            rom_byte_mask = 1U;
        }
    } while (rom_byte_number < 8);

    /* Erfolg: alle 64 Bits verarbeitet UND CRC ueber alle 8 Bytes ergibt 0 */
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

/** Neue Suche starten – liefert ROM des ersten Sensors (oder false) */
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

/** Naechsten Sensor suchen – State von vorherigem Lauf wird weiterverwendet */
bool ow_search_next(uint8_t rom[8]) {
    if (!ow_search_run()) {
        return false;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        rom[i] = ROM_NO[i];
    }

    return true;
}
