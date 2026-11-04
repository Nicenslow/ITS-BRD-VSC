# Zeitverhalten

Der Timer zählt 90-mal je Mikrosekunde. Ein Überlauf des 32-Bit-Registers liegt bei etwa 47 s. `timebase_now` addiert jede Differenz in einen 64-Bit-Zähler und wird in jedem Super-Loop aufgerufen. Steht der Drehgeber länger als 47 s, bleibt die Spanne zwischen zwei Wechseln trotzdem richtig.

Die LCD-Ausgabe ist der langsame Teil. Beschriftungen entstehen nur in `display_init`. Winkel und Geschwindigkeit liegen in einem Puffer und verlassen ihn mit einem Zeichen je Durchlauf. D20 geht unmittelbar vor `lcdPrintC` auf high und direkt danach auf low. Die Impulsbreite am Oszilloskop ist die Zeit für ein Zeichen.

Zum Vergleich mit einem ganzen Super-Loop ohne Zeichen: D20 im Labor probeweise am Anfang des Durchlaufs setzen und am Ende löschen. Diese Messung ist deutlich kürzer als ein Zeichen. Ein voller Zahlenstring ist etwa so lang wie die Zeichenzahl mal der Einzelimpuls.

Neue Zahlen werden frühestens alle 300 ms berechnet. Fehlt in diesem Fenster ein Phasenwechsel, wartet das Programm auf den nächsten Wechsel, höchstens aber 3 s. Bei 0,1 Grad/s liegt zwischen zwei Phasen genau 3 s, weil ein Schritt 0,3 Grad ist. Vorher 0 anzuzeigen wäre gröber als diese Grenze.

Die Geschwindigkeit selbst benutzt nicht die 300 ms, sondern die Zeitstempel der beiden Phasenzählerstände. Die liegen auf den Wechseln und nicht auf dem Zeitpunkt der LCD-Ausgabe.

Die konkrete Oszilloskop-Messung von Zeichenzeit und Schleifenzeit wird im Labor in diese Datei eingetragen.
