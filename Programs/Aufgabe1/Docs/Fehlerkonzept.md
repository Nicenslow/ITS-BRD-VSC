# Fehlerkonzept

Der Rechner unterscheidet den Fehlerstatus von einem Zahlenwert. Eine Funktion, die eine Zahl liefert, schreibt diese in einen Zeigerparameter. Der Rückgabewert ist ausschließlich ein `Result` aus `Inc/result.h`.

Damit bleibt zum Beispiel `stack_pop` eindeutig: Liegt oben `-7`, kommt `RESULT_OK` zurück und `*out_value` ist `-7`. Ist der Stack leer, kommt `ERROR_STACK_UNDERFLOW` zurück. Ein negativer Rückgabewert als Fehlercode würde diese beiden Fälle vermischen. Deshalb werden weder der `errno`-Weg noch ein negativer POSIX-Code für den Zahlenwert benutzt.

## Codes

| Code | Bedeutung | Sichtbarer Text |
| --- | --- | --- |
| `RESULT_OK` | Operation gelungen | kein Fehlertext |
| `ERROR_STACK_OVERFLOW` | `push` auf vollen Stack, auch bei `d` | `Error: stack overflow` |
| `ERROR_STACK_UNDERFLOW` | zu wenig Operanden für `+ - * / p P d r` | `Error: stack underflow` |
| `ERROR_DIV_ZERO` | Division durch 0 | `Error: division by zero` |
| `ERROR_ARITHMETIC_OVERFLOW` | Ergebnis passt nicht in `signed int` | `Error: arithmetic overflow` |
| `ERROR_INPUT_OVERFLOW` | eingegebene Ziffernfolge passt nicht in `signed int` | `Error: input overflow` |

`calc_error_print` schaltet das obere Terminal auf Rot, löscht es und schreibt genau eine dieser Zeilen. Der Code liegt nicht in `error.c` der Board-Bibliothek, weil dort `Error_Handler` für die LEDs definiert ist.

## Weiterreichen

Jede Schicht gibt den Status nach oben weiter:

- `stack_*` erkennt Über- und Unterlauf.
- `arithmetic_*` erkennt Division durch 0 und Bereichsüberschreitung und reicht Stackfehler durch.
- Der Scanner liefert bei einer zu großen Ziffernfolge das Token `OVERFLOW`. `main` macht daraus `ERROR_INPUT_OVERFLOW`.
- `main` zeigt den Status an und merkt sich, dass ein Fehler anliegt.

Schlägt eine Rechen- oder Stackoperation fehl, werden bereits entnommene Werte zurückgelegt. Der Stack hat danach denselben Inhalt wie vor dem Befehl. Sichtbar ist das erst nach dem Quittieren nicht mehr, weil `C` den Rechner neu initialisiert.

## Nach einem Fehler

Solange ein Fehler anliegt, werden alle Tasten außer `C` ignoriert. `C` löscht den Stack, löscht beide Terminals, stellt die normale Textfarbe wieder her und gibt `Reset.` aus. Danach nimmt der Rechner wieder Eingaben an.

`C` ohne vorherigen Fehler löscht Stack und Terminals ebenfalls, gibt aber keinen Text aus.

## Bereichsüberschreitung

Die eigentliche Operation wird nur ausgeführt, wenn sie in `signed int` bleibt. Geprüft wird vorher, nicht am umgebrochenen Ergebnis.

Addition `a + b`:

- `b > 0` und `a > INT_MAX - b`, oder
- `b < 0` und `a < INT_MIN - b`

Subtraktion `a - b`:

- `b < 0` und `a > INT_MAX + b`, oder
- `b > 0` und `a < INT_MIN + b`

Multiplikation, `a` und `b` ungleich 0:

- beide positiv: `a > INT_MAX / b`
- `a` positiv, `b` negativ: `b < INT_MIN / a`
- `a` negativ, `b` positiv: `a < INT_MIN / b`
- beide negativ: `a < INT_MAX / b` (das Produkt wäre positiv)

Ist ein Faktor 0, gibt es keinen Überlauf. Die Division `INT_MIN / -1` ist ein Überlauf, `INT_MAX / -1` nicht. Die ganzzahlige Division rundet gegen 0.
