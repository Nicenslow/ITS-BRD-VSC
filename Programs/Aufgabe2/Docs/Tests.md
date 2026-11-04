# Tests Aufgabe 2

Die Folgen beschreiben die Phasen, die der Super-Loop nacheinander sieht. Die erste Phase ist die Synchronisation und zählt nicht. Der PC-Test `tests/test_motion.c` prüft dieselben Zählerstände.

## Normalverhalten

| Nr | Folge | Ergebnis |
| --- | --- | --- |
| 1 | a, b, c, d, a | Zähler 4, D23 an, Winkel 1,2 Grad |
| 2 | b, a | Zähler −1, D22 an, Winkel −0,3 Grad |
| 3 | a, b, b, b, c | Zähler 2, die wiederholte Phase b ändert nichts |
| 4 | 1200 Vorwärtsschritte nach der Synchronisation | Winkel 360,0 Grad, untere 8 Bit der LEDs sind 0 |
| 5 | vier Schritte innerhalb einer Sekunde | Geschwindigkeit 1,2 Grad/s |
| 6 | drei Sekunden keine neue Phase | Geschwindigkeit 0,0 Grad/s, Winkel bleibt |
| 7 | Schritte, dann 300 ms Pause, dann weiter | Display übernimmt den neuen Stand erst nach mindestens 300 ms |
| 8 | S6 nach ein paar Schritten | Zähler 0, beide Richtungs-LEDs aus, Anzeige 0,0 |

## Fehler

| Nr | Folge | Ergebnis |
| --- | --- | --- |
| 9 | b, dann d | D21 an, Zähler bleibt 0, Display und Zähler-LEDs ändern sich nicht mehr |
| 10 | a, b, dann d | D21 an, Zähler bleibt 1 |
| 11 | nach Fall 9 noch c und a | weiter Fehler, Zähler bleibt 0 |
| 12 | nach Fall 9 die Taste S6, danach b, c | Fehler aus, Zähler 1, D23 an |

Am Board zusätzlich langsam drehen und an den Eingangs-LEDs die Phasen a, b, c, d mitlesen. Dieselbe Folge mit dem Rechteckgenerator auf dem Pi Pico wiederholen. Die schnelle Drehung muss D21 setzen. Das ist die erwartete Grenze des Pollings.
