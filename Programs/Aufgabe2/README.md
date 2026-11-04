# Aufgabe 2 – Drehgeber per Polling

Super-Loop für den inkrementellen Drehgeber auf dem ITS-Board. Kanal A liegt an IN0 (PF0), Kanal B an IN1 (PF1). Die Taste S6 (PF6, low-aktiv) setzt das Programm zurück.

## Projekt öffnen

1. Im CMSIS-View `Programs/Aufgabe2/Aufgabe2.csolution.yml` als Solution wählen.
2. Einmal bauen. Dabei entsteht die `compile_commands.json` für den Linter.
3. Mit Load & Run auf das Board übertragen.

## Anzeige

Die unteren 8 Bit des Phasenzählers stehen binär auf D8 bis D15. D23 leuchtet nach dem letzten Vorwärtsschritt, D22 nach dem letzten Rückwärtsschritt. D21 leuchtet im Fehlerfall. Danach bleiben Zähler-LEDs, Richtung und Display stehen, bis S6 gedrückt wird.

Auf dem LCD stehen Winkel in Grad und Winkelgeschwindigkeit in Grad je Sekunde. Die Beschriftung wird nur beim Start geschrieben. Neue Zahlen gehen zeichenweise hinaus, ein Zeichen je Super-Loop. D20 (PE4) ist dabei high und dient der Messung am Oszilloskop.

## Unterlagen

- `Docs/Modulkonzept.md`
- `Docs/Fehlerkonzept.md`
- `Docs/Eingabezustaende.md`
- `Docs/Tests.md`
- `Docs/Zeitverhalten.md`

Der PC-Test für Phasenfolge und Winkel liegt in `tests/test_motion.c`.
