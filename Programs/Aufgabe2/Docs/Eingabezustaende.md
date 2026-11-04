# Eingabezustände

Gelesen wird nur die Phase aus Kanal A und B und die Taste S6. Die Reaktion hängt von der letzten gültigen Phase ab.

| Aktuell | Neu | Ergebnis |
| --- | --- | --- |
| a | b | vorwärts, Zähler +1 |
| b | c | vorwärts, Zähler +1 |
| c | d | vorwärts, Zähler +1 |
| d | a | vorwärts, Zähler +1 |
| a | d | rückwärts, Zähler −1 |
| d | c | rückwärts, Zähler −1 |
| c | b | rückwärts, Zähler −1 |
| b | a | rückwärts, Zähler −1 |
| gleich | gleich | keine Änderung |
| b | d | Fehler, Zähler und Anzeige bleiben stehen |
| a | c | Fehler, Zähler und Anzeige bleiben stehen |

Die erste Messung nach Start oder Reset übernimmt nur die Phase. Sie zählt nicht als Wechsel.

S6, Flanke von losgelassen nach gedrückt: Zähler 0, Richtung aus, D21 aus, Anzeige 0,0 Grad und 0,0 Grad/s. Die aktuelle Phase wird der neue Ausgangspunkt.

Solange der Fehler steht, ändern weitere Phasen nichts. Erst S6 hebt den Zustand auf.
