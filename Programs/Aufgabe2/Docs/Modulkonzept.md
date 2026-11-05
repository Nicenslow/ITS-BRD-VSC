# Modulkonzept

Ein Kasten ist ein C-Modul, also eine `*.h` und eine `*.c`. Das Modul gibt es einmal im Programm. `+` ist von anderen Modulen benutzbar, `-` nur innerhalb des Moduls. Oberhalb des Strichs stehen Typen und Variablen, unterhalb die Funktionen. Der Pfeil zeigt auf das benutzte Modul. Die Beschriftung nennt, was davon gebraucht wird. Kein Modul liest die mit `-` markierten Daten eines anderen Moduls.

`phase` hält nur die gemeinsamen Typen. `timer` und `lcd` sind Module der Board-Bibliothek.

```mermaid
%%{init: {"theme": "base", "themeVariables": {"fontFamily": "Consolas, Courier New, monospace", "fontSize": "14px", "mainBkg": "#ffffff", "nodeBorder": "#222222", "lineColor": "#222222", "textColor": "#111111"}}}%%
classDiagram
direction TB

class phase["&lt;File&gt; phase"] {
    + enum Phase: PHASE_A, PHASE_B, PHASE_C, PHASE_D
    + struct PhaseSample: Phase phase; uint64_t timestamp
    + enum Direction: DIRECTION_UNKNOWN, DIRECTION_FORWARD, DIRECTION_BACKWARD
}

class timebase["&lt;File&gt; timebase"] {
    - uint32_t last_raw_count
    - uint64_t accumulated_ticks
    + const uint64_t DISPLAY_INTERVAL_TICKS
    + const uint64_t DISPLAY_TIMEOUT_TICKS
    + void timebase_init(void)
    + uint64_t timebase_now(void)
    + double timebase_seconds_between(uint64_t start_tick, uint64_t end_tick)
}

class gpio_in["&lt;File&gt; gpio_in"] {
    - const uint32_t CHANNEL_A_PIN
    - const uint32_t CHANNEL_B_PIN
    - const uint32_t RESET_BUTTON_PIN
    + void gpio_in_init(void)
    + PhaseSample gpio_in_read_phase(void)
    + bool gpio_in_read_reset_button(void)
}

class motion["&lt;File&gt; motion"] {
    - Phase last_phase
    - int32_t phase_count
    - uint64_t change_timestamp
    - Direction direction
    - bool has_error
    - bool is_synced
    + void motion_init(void)
    + void motion_reset(PhaseSample sample)
    + void motion_update(PhaseSample sample)
    + bool motion_has_error(void)
    + int32_t motion_count(void)
    + uint64_t motion_change_timestamp(void)
    + Direction motion_direction(void)
}

class angle["&lt;File&gt; angle"] {
    + struct CountSample: int32_t count; uint64_t timestamp
    - const double DEGREES_PER_PHASE
    + const int PHASES_PER_REVOLUTION
    + double angle_degrees(int32_t phase_count)
    + double angle_velocity_degrees_per_second(CountSample start, CountSample end)
}

class display["&lt;File&gt; display"] {
    - struct OutputLine: char text[17]; int length; int next_index; POINT row
    - OutputLine output_lines[2]
    - int active_line
    - void format_line(OutputLine* line, double value)
    + void display_init(void)
    + void display_submit(double angle_deg, double velocity_deg_per_sec)
    + bool display_is_busy(void)
    + void display_pump(void)
}

class gpio_out["&lt;File&gt; gpio_out"] {
    - const uint32_t COUNT_LED_MASK
    - const uint32_t MEASURE_PIN
    - const uint32_t ERROR_LED_PIN
    - const uint32_t BACKWARD_LED_PIN
    - const uint32_t FORWARD_LED_PIN
    - void write_pin(uint32_t pin, bool is_high)
    + void gpio_out_init(void)
    + void gpio_out_show_count(int32_t phase_count)
    + void gpio_out_show_direction(Direction direction)
    + void gpio_out_show_error(bool is_on)
    + void gpio_out_set_measure_pin(bool is_high)
}

class main["&lt;File&gt; main"] {
    - bool reset_was_down
    - CountSample published
    - uint64_t last_display_tick
    + int main(void)
    - void reset_program(PhaseSample sample)
    - void publish_measurement(CountSample start, bool count_changed)
}

class timer["&lt;File&gt; timer"] {
    + uint32_t getTimeStamp(void)
}

class lcd["&lt;File&gt; lcd"] {
    + void lcdPrintS(char* s)
    + void lcdPrintC(char c)
}

main --> gpio_in : Phase und S6
main --> motion : Zähler aktualisieren
main --> angle : Grad und Geschwindigkeit
main --> display : LCD-Puffer
main --> gpio_out : LEDs
main --> timebase : 300 ms und 3 s
gpio_in --> phase : PhaseSample
gpio_in --> timebase : Zeitstempel
motion --> phase : Phase und Direction
gpio_out --> phase : Direction
angle --> timebase : Zeit in Sekunden
timebase --> timer : getTimeStamp
display --> gpio_out : Messpin
display --> lcd : ein Zeichen
```

## Verhalten

`timebase_now` addiert die vorzeichenlose Differenz des Timerregisters in `accumulated_ticks`. Die Zeit bleibt damit nach dem Überlauf des 32-Bit-Zählers richtig. `gpio_in_read_phase` liest PF0 und PF1 und hängt den Zeitstempel direkt an die Phase. S6 ist low-aktiv.

`motion_update` zählt einen Nachbarwechsel vorwärts als +1 und rückwärts als -1. Dieselbe Phase ändert nichts. Ein Sprung setzt `has_error`; Zähler und Zeitstempel bleiben stehen. `motion_reset` übernimmt die aktuelle Phase neu und setzt den Zähler auf 0.

1200 Phasenwechsel sind eine Umdrehung. Winkel und Geschwindigkeit rechnen mit `double`. Ohne neuen Wechsel ist die Geschwindigkeit 0. `display_init` schreibt die Beschriftungen einmal. `display_pump` gibt pro Aufruf höchstens ein Zeichen aus. Die unteren 8 Bit des Zählers liegen auf PD0 bis PD7, Richtung, Fehler und Messpegel auf PE4 bis PE7.

`main` liest am Anfang des Durchlaufs Phase und S6 und arbeitet danach nur mit diesen Werten. Neue LCD-Zahlen gibt es erst bei leerem Puffer, frühestens 300 ms nach der letzten Ausgabe, und ohne Phasenwechsel spätestens nach 3 s.

## Schnitt

Die Register von Drehgeber, Taster, LEDs und Timer liegen in `gpio_in`, `gpio_out` und `timebase`. `motion` ist der Zustand des Drehgebers. `angle` rechnet nur. `display` ist die Ausgabe. `main` verbindet diese Teile und hält keine Kopien der versteckten Zähler.
