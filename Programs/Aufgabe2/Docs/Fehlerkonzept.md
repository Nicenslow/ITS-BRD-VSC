# Fehlerkonzept

Es gibt einen Fehler: Zwischen zwei Messungen wurde eine Phase übersprungen. Der Drehgeber ist dann von einer Phase in die übernächste gesprungen, zum Beispiel von b nach d.

`motion_update` setzt dafür ein Bit. Der Phasenzähler, sein Zeitstempel und die letzte gültige Richtung bleiben unverändert. Weitere Phasen liest das Modul zwar noch ein, wertet sie aber nicht mehr aus.

`main` schaltet D21 ein und schreibt Zähler-LEDs, Richtungs-LEDs und Display nicht mehr. Der zuletzt sichtbare Stand bleibt stehen.

S6 setzt das ganze Programm zurück, nicht nur das Fehlerbit. Zähler, Richtung und Anzeige beginnen wieder bei 0. Die Taste wirkt auf die Flanke, damit ein gehaltenes S6 den Durchlauf nicht in jedem Umlauf neu startet.

Andere Module geben keinen Fehlercode zurück. GPIO-Lesen und die Winkelrechnung haben keine eigene Fehlerquelle. Der eine Zustand liegt in `motion` und wird von `main` auf die Ausgänge gelegt.
