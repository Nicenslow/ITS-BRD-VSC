# Bericht – Aufgabe 4: 1-Wire Temperaturmessung

## Konzept und Architektur

Das System ist in sechs Anwendungsmodule und die Board-Bibliothek unterteilt. Die Abhaengigkeiten bilden eine Schichtenstruktur:

```text
main
 ├── display_temp  → lcd.h / LCD_GUI (Board-Library)
 ├── ow_search     → 1wire, crc8
 ├── ds18x20       → 1wire, crc8, timer_util
 └── 1wire         → timer_util, GPIO (HAL)
      └── timer_util → timer.h (TIM2, ITS_BRD_LIB)
```

- **timer_util** kapselt alle Wartezeiten (Micro-/Millisekunden) ueber TIM2; es gibt keine leeren Busy-Wait-Schleifen ohne Timerbezug.
- **crc8** validiert ROM- und Scratchpad-Daten (Polynom gemaess AN27, Lookup-Tabelle).
- **1wire** implementiert das Master-Timing auf PD0; PD1 bleibt fuer parasitaere Versorgung aktiv.
- **ds18x20** kapselt Match-ROM, Convert-T und Scratchpad-Auswertung fuer DS18B20 und DS18S20.
- **ow_search** enthaelt als einziges Modul globalen Search-State (AN187).
- **display_temp** zeigt Ergebnisse und Fehlertexte auf Deutsch an.

## Implementierungsentscheidungen

| Entscheidung | Begruendung |
|--------------|-------------|
| Lookup-Tabelle fuer CRC-8 | Geringer CPU-Aufwand bei jedem ROM-/Scratchpad-Read; Tabelle aus AN27, 256 Byte Flash |
| PD0 als Open-Drain | Entspricht 1-Wire-Bus-Eigenschaften; PD1 push-pull fuer Versorgung |
| Starker Pullup ueber PD0+PD1 | Gemaess Aufgabenstellung bei parasitaerer Versorgung waehrend Convert T |
| Konversionszeit 750 ms | Maximale Zeit fuer alle DS18B20-Aufloesungen laut Datenblatt |
| Search exakt nach AN187 | Vorgabe der Aufgabe; Discrepancy-Tracking und CRC8 im Search-Lauf |
| Teilaufgaben per `AUFGABE4_TEILAUFGABE` | Eine Codebasis, nachvollziehbare Abgabe jeder Teilaufgabe |

## Testfaelle

### Test 1: Einzelner Sensor – ROM lesen, CRC pruefen

- **Vorbereitung:** Ein DS18B20 an PD0/PD1, `AUFGABE4_TEILAUFGABE = 1`.
- **Schritte:** Programm starten, Display beobachten.
- **Erwartung:** 16-stelliger Hex-ROM-Code (Family-Byte `28` am Anfang); Wert bleibt stabil; kein CRC-Fehler.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 2: Kein Sensor – Fehlermeldung

- **Vorbereitung:** Kein Sensor am Bus, beliebige Teilaufgabe.
- **Schritte:** Reset ohne angeschlossenen Sensor.
- **Erwartung:** Anzeige **Kein Sensor gefunden**, zyklische Wiederholung ohne Absturz.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 3: Sensor entfernen waehrend Betrieb

- **Vorbereitung:** Teilaufgabe 3, ein Sensor angeschlossen, laeuft stabil.
- **Schritte:** Sensor waehrend Messzyklus abziehen.
- **Erwartung:** Wechsel zu **Kein Sensor gefunden**; nach Wiederanschluss automatische Wiederaufnahme.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 4: Mehrere Sensoren – Search findet alle

- **Vorbereitung:** 2–3 DS18B20 parallel am Bus, `AUFGABE4_TEILAUFGABE = 3`.
- **Schritte:** Liste auf Display pruefen, Sensoren einzeln identifizieren (ROM unterscheidet sich).
- **Erwartung:** Jeder Sensor eine Zeile mit Index, ROM, Temperatur; Anzahl = angeschlossene Sensoren.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 5: CRC-Fehler

- **Vorbereitung:** Stoerung simulieren (loses Kabel, Bus waehrend Uebertragung kurz stoeren) oder absichtlich fehlerhafte ROM-Konstante in Teilaufgabe 2.
- **Schritte:** Messzyklus ausloesen.
- **Erwartung:** **CRC-Fehler - Messung wiederholen**, naechster Zyklus erneuter Versuch.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 6: Temperaturgenauigkeit

- **Vorbereitung:** Teilaufgabe 2 oder 3, Raumtemperatur als Referenz.
- **Schritte:** Kaeltespray (kurz, vorsichtig) bzw. warmen Metallloeffel an Sensor halten.
- **Erwartung:** Anzeige faellt bzw. steigt sichtbar; nach Entfernung der Quelle langsame Rueckkehr zum Ausgangswert.
- **Ergebnis:** *(am Praktikum ausfuellen)*

### Test 7: DS18B20 vs. DS18S20

- **Vorbereitung:** Sensor mit Family-Code `0x28` bzw. `0x10` (ROM-Byte 0).
- **Schritte:** ROM auf Display pruefen, Temperatur bei Raumtemperatur vergleichen.
- **Erwartung:** DS18B20: Aufloesung 0,0625 °C; DS18S20: 0,5 °C-Schritte; korrekte Umrechnung in `ds18x20.c`.
- **Ergebnis:** *(am Praktikum ausfuellen)*

## Bekannte Einschraenkungen

- Maximal **8 Sensoren** gleichzeitig in der Anzeige (`DISPLAY_TEMP_MAX_SENSORS`); bei mehr Geraeten am Bus werden nur die ersten acht dargestellt.
- Teilaufgabe 2 erfordert manuelles Eintragen der ROM-Codes in `s_known_roms[]`.
- Sehr lange ROM-Zeilen auf dem 480×320-Display mit Schriftgroesse 12; bei vielen Sensoren kann der untere Rand abgeschnitten werden.
- Timing haengt von TIM2 (90 Ticks/µs bei 180 MHz) ab; Abweichungen der Systemtaktung wuerden das 1-Wire-Timing beeinflussen.
- Open-Drain auf PD0 setzt voraus, dass PD1 den Pullup ueber 3,3 kΩ bereitstellt.
