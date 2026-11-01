# Tests Aufgabe 1

Enter ist die unbeschriftete Taste. Sie steht in den Folgen nur zwischen zwei Zahlen. Operatoren und Buchstaben brauchen kein Enter davor.

`p` schreibt eine Zahl und einen Zeilenumbruch. `P` schreibt bis zu drei oberste Werte in einer Zeile, durch Leerzeichen getrennt, der oberste Wert am Ende. Sind weniger als drei Werte da, werden alle gezeigt. Danach folgt ein Zeilenumbruch.

Die Fälle der Bereiche und der Stackgrenzen prüft zusätzlich `tests/test_logic.c` auf dem PC. Am Board zählt die sichtbare Meldung.

## Normalverhalten

| Nr | Eingabe | Reaktion |
| --- | --- | --- |
| 1 | `2` `5` Enter `8` Enter `6` `7` `+` `r` `/` `p` | Ausgabe `3`. Rechnung: `8+67=75`, Tausch ergibt obersten Wert `25`, `75/25=3`. |
| 2 | `2` Enter `3` `+` `p` | Ausgabe `5` |
| 3 | `1` `0` Enter `3` `-` `p` | Ausgabe `7` |
| 4 | `3` Enter `1` `0` `-` `p` | Ausgabe `-7` |
| 5 | `6` Enter `7` `*` `p` | Ausgabe `42` |
| 6 | `2` `0` Enter `4` `/` `p` | Ausgabe `5` |
| 7 | `7` Enter `2` `/` `p` | Ausgabe `3` |
| 8 | `3` Enter `1` `0` `-` Enter `4` `+` `p` | Ausgabe `-3` |
| 9 | `5` `p` `p` | zweimal `5`, der Stack enthält danach weiter `5` |
| 10 | `5` `d` `P` | Ausgabe `5 5` |
| 11 | `2` Enter `9` `r` `p` | Ausgabe `2` |
| 12 | `2` Enter `9` `r` `P` | Ausgabe `9 2` |
| 13 | `1` Enter `2` Enter `3` Enter `4` `P` | Ausgabe `2 3 4`, die `1` bleibt auf dem Stack |
| 14 | `8` Enter `1` `P` | Ausgabe `8 1` |
| 15 | `4` `P` | Ausgabe `4` |
| 16 | `1` Enter `2` `C` `p` | `Error: stack underflow`, der Stack ist leer |
| 17 | `0` Enter `5` `/` `p` | Ausgabe `0` |
| 18 | `7` Enter `0` Enter `1` `-` `/` `p` | `7 / -1`, Ausgabe `-7` |

## Fehler

Nach jedem Fehler muss `C` gedrückt werden, bevor die nächste Folge gilt. Andere Tasten ändern bis dahin nichts. `C` löscht die Meldung und schreibt `Reset.`

| Nr | Eingabe | Reaktion |
| --- | --- | --- |
| 19 | `+` | `Error: stack underflow` |
| 20 | `1` `+` | `Error: stack underflow` |
| 21 | `1` `r` | `Error: stack underflow` |
| 22 | `P` auf leerem Stack, direkt nach `C` | `Error: stack underflow` |
| 23 | `5` Enter `0` `/` | `Error: division by zero` |
| 24 | `1` `+` `5` `p` | weiter die Underflow-Meldung, die `5` wird nicht übernommen. Danach `C`, dann `5` `p` gibt `5` aus. |
| 25 | 33-mal die Zahl `1`, jeweils durch Enter getrennt | die 33. Zahl: `Error: stack overflow`. Der Stack fasst 32 Werte. |
| 26 | Stack mit 32 Werten füllen, dann `d` | `Error: stack overflow` |
| 27 | `2` `1` `4` `7` `4` `8` `3` `6` `4` `8` | schon während der Ziffern: `Error: input overflow` |
| 28 | `2` `1` `4` `7` `4` `8` `3` `6` `4` `7` Enter `1` `+` | `Error: arithmetic overflow` |
| 29 | `0` Enter `2` `1` `4` `7` `4` `8` `3` `6` `4` `7` `-` Enter `1` `-` Enter `1` `-` | erst `-2147483647`, dann `INT_MIN`, dann `Error: arithmetic overflow` |
| 30 | `1` `0` `0` `0` `0` `0` Enter `1` `0` `0` `0` `0` `0` `*` | `Error: arithmetic overflow` |
| 31 | `0` Enter `2` `1` `4` `7` `4` `8` `3` `6` `4` `7` `*` `p` | Ausgabe `0` |
| 32 | `0` Enter `2` `1` `4` `7` `4` `8` `3` `6` `4` `7` `-` Enter `1` `-` Enter `0` Enter `1` `-` `/` | Stack enthält `INT_MIN` und `-1`, dann `Error: arithmetic overflow` |
| 33 | `2` `1` `4` `7` `4` `8` `3` `6` `4` `7` Enter `0` Enter `1` `-` `/` `p` | Ausgabe `-2147483647` |

Fall 29 im Einzelnen: `0 - 2147483647 = -2147483647`, minus `1` ergibt `-2147483648`, minus `1` passt nicht mehr in `signed int`.

Fall 32 im Einzelnen: derselbe Weg bis `INT_MIN`, danach `0 - 1 = -1`, dann `INT_MIN / -1`.

Fall 33 zeigt die Grenze daneben: `INT_MAX / -1` ist darstellbar.
