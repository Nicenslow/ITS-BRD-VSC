# Aufgabe 4 – 1-Wire Temperaturmessung (DS18B20 / DS18S20)

Temperaturmessung ueber den 1-Wire-Bus mit parasitaerer Stromversorgung. Das Programm unterstuetzt drei aufeinander aufbauende Teilaufgaben, die ueber `Inc/config.h` (`AUFGABE4_TEILAUFGABE`) gewaehlt werden.

## Hardware-Aufbau

| Pin  | Funktion                                      |
|------|-----------------------------------------------|
| PD0  | 1-Wire-Datenleitung (DQ)                      |
| PD1  | Dauerhaft logisch 1 (3,3 V), parasitaere Versorgung |
| —    | 3,3 kΩ zwischen PD1 und 1-Wire-Bus (Strombegrenzung) |

**Sensoren:** DS18B20 (Family-Code `0x28`) oder vereinzelt DS18S20 (`0x10`). Alle GND-Pins der Sensoren mit Board-GND verbinden. DQ aller Sensoren am gemeinsamen 1-Wire-Bus (PD0). Parasitische Versorgung: kein separates VDD noetig.

**Waehrend der Temperaturmessung** ziehen PD0 und PD1 gleichzeitig High (starker Pullup) fuer die gesamte Konversionszeit (750 ms).

## Module

| Modul          | Dateien              | Aufgabe                                      |
|----------------|----------------------|----------------------------------------------|
| `timer_util`   | timer_util.c/h       | Mikro-/Millisekunden-Wartezeiten (TIM2)      |
| `crc8`         | crc8.c/h             | 1-Wire-CRC (Lookup-Tabelle AN27)             |
| `1wire`        | 1wire.c/h            | Bus-Reset, Bit-/Byte-Zugriff                 |
| `ds18x20`      | ds18x20.c/h          | ROM, Konvertierung, Temperatur               |
| `ow_search`    | ow_search.c/h        | Search-Algorithmus (AN187)                   |
| `display_temp` | display_temp.c/h     | LCD-Ausgabe (Waveshare 4")                   |
| `main`         | main.c, config.h     | Endlosschleife je Teilaufgabe                |

## Teilaufgaben und Code

In `Inc/config.h` den Wert `AUFGABE4_TEILAUFGABE` setzen:

| Wert | Beschreibung | Code in `main.c`        |
|------|--------------|-------------------------|
| 1    | Ein Sensor, ROM zyklisch lesen | `run_teilaufgabe1()` |
| 2    | Mehrere Sensoren mit festen ROM-Codes | `run_teilaufgabe2()` – ROMs in `s_known_roms[]` eintragen |
| 3    | Automatische Suche + Messung (Standard) | `run_teilaufgabe3()` |

Standard ist **Teilaufgabe 3**.

## Kompilieren und Flashen

1. CMSIS-Toolbox / VS Code: Solution `Programs/Aufgabe4/Aufgabe4.csolution.yml` oeffnen.
2. Build-Konfiguration **Debug + ITSboard** waehlen und bauen.
3. Mit pyOCD / ST-Link flashen (analog zu Aufgabe 2/3).

Alternativ im Ordner `Programs/Aufgabe4`:

```text
cbuild Aufgabe4.csolution.yml
```

## Fehlermeldungen auf dem Display

- **Kein Sensor gefunden** – kein Presence-Puls nach Reset
- **CRC-Fehler - Messung wiederholen** – ungueltiger ROM- oder Scratchpad-CRC
