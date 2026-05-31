# Anleitung: Echter Drehgeber vs. Pi-Pico-Rechteckgenerator

Diese Anleitung ergänzt die offizielle PDF *Beschreibung_Rechteckgenerator_PiPico.pdf* und die Aufgabenstellung zu Aufgabe 2 (ITS-Board: Kanäle **IN0 / IN1** = **GPIO PF0 / PF1**, gemeinsame **Masse** mit dem Pico bzw. Drehgeber).

---

## Teil A – Echter inkrementaler Drehgeber am ITS-Board

1. **Versorgung:** Nur **3,3 V** vom ITS-Board verwenden (nicht 5 V), so wie in der Aufgabenstellung beschrieben.
2. **Signale:** Ausgang **Kanal A** → **IN0 (PF0)**, **Kanal B** → **IN1 (PF1)**.
3. **Masse:** **GND** Drehgeber mit **GND** des ITS-Systems verbinden (Pflicht für saubere Quadratursignale).
4. **Firmware:** Dein Programm *Aufgabe 2* auf das STM32-Board flashen und starten.
5. **Test:** Langsam drehen – die **LEDs zu IN0/IN1** zeigen die beiden Kanäle; der **binäre Phasenzähler** (LEDs D8–D15 / PD0–7) und das **LCD** sollten mitlaufen. Bei sehr schnellem Drehen können Phasen „übersehen“ werden → Fehleranzeige laut Konzept.

Es gibt **keinen PC-Rechteckgenerator**: Du bedienst nur das ITS-Board und den Drehgeber.

---

## Teil B – Simulation mit Raspberry Pi Pico (Rechteckgenerator)

Der Pico erzeugt auf zwei Pins zwei rechteckförmige Signale im Quadratur-Abstand (wie ein Drehgeber). Die **Steuerung** (Winkel, Geschwindigkeit, Start/Stopp) läuft vom **PC** über **USB-Seriell** mit der mitgelieferten **Python-GUI** oder einem Terminal.

### B1 Firmware auf den Pico laden

1. Pico per USB mit dem PC verbinden.
2. **Bootloader:** Großen **BOOTSEL**-Taster auf dem Pico gedrückt halten, USB einstecken, dann loslassen. Der Pico erscheint als **USB-Laufwerk** („RPI-RP2“).
3. Die Datei **`pio_2_squarewaves.uf2`** (vom Rechteckgenerator-Paket, z. B. über Emil bereitgestellt) auf dieses Laufwerk **kopieren**.
4. Der Pico startet neu und führt das Programm aus.

### B2 Elektrischer Anschluss Pico ↔ ITS-Board

- Signale laut PDF auf den Pico-Pins **A0** und **A1** (exakte physikalische Lage: Anschlussbild in der PDF *Beschreibung_Rechteckgenerator_PiPico* bzw. eurem Laboraufbau).
- Diese beiden Leitungen mit **IN0** und **IN1** des ITS-Boards verbinden (Zuordnung zu Kanal A/B wie im Labor festgelegt; konsistent halten).
- **GND Pico** mit **GND ITS** verbinden.
- Spannungspegel beachten: **3,3-V-Logik** – keine 5-V-Pegel auf STM32-Eingänge legen.

Danach kann parallel das STM32-Programm *Aufgabe 2* laufen und die simulierten Flanken auswerten.

### B3 Python-GUI auf dem PC

**Voraussetzungen**

- **Python 3** (in der Anleitung z. B. 3.9 genannt; aktuelle 3.11/3.12 gehen meist auch).
- Pakete (in einer Shell **im Projektordner der GUI** oder global):

  ```text
  pip install pyserial colour
  ```

  (`colour` schreibt sich auf pip oft genau so.)

**Start**

1. In den Ordner wechseln, der **`main.py`** enthält (bei dir typisch  
   `...\Rechteckgenerator\Python_GUI`).
2. Start:

   ```text
   python main.py
   ```

   Unter Windows ggf. `py main.py` oder `python3 main.py`.

**Bedienung**

1. **`Connect`:** Serielle Schnittstelle wählen, die zum **USB des Pico** gehört (oft eine **COM-Port mit hoher Nummer**). Ohne Verbindung keine weiteren Befehle.
2. **Winkel** (`angle [°]`): Nur gültige Werte (siehe **`?`** am Feld): u. a. durch **0,3°** teilbar, Bereich und Dezimalstellen wie in der Hilfe – **keine Änderung**, solange der Generator **läuft**.
3. **Winkelgeschwindigkeit** (`angular velocity [°/s]`): Ganzzahl, weitere Regeln stehen hinter **`?`** (u. a. Teilerbedingung aus der Implementierung).
4. **`Start`:** Sendet Winkel und Geschwindigkeit an den Pico und startet die Ausgabe. Status zeigt **running** / Ampelfarbe.
5. **`Stop`:** Generator stoppen; danach wieder Winkel/Geschwindigkeit änderbar.

Tipps aus der Implementierung: Bei Fehlermeldungen oft **Pico neu einstecken oder neu flashen**; falsche COM-Port-Wahl führt zu Timeout oder „nicht der Pico“.

### B4 Alternative: Terminal (z. B. Tera Term)

1. Serielle Verbindung zum **gleichen COM-Port** wie der Pico.
2. Nach Verbindung eine **Taste drücken**, bis das Menü erscheint (wie PDF).
3. Unter **Terminal-Einstellungen** „**Lokales Echo**“ aktivieren, damit Eingaben sichtbar sind.
4. Befehle für Geschwindigkeit/Winkel und Start/Stopp wie auf dem Pico dokumentiert; **`h`** für Hilfe.

---

## Kurzüberblick

| Situation | Was du tun musst |
|-----------|-------------------|
| **Echter Drehgeber** | 3,3 V, GND, A/B auf PF0/PF1; STM32-Programm flashen; drehen und Auswertung prüfen. |
| **Pi Pico** | UF2 flashen; A0/A1 → IN0/IN1, GND gemeinsam; Python-GUI: `pip install pyserial colour`, dann `python main.py` → Connect → Parameter → Start/Stop. |

Bei Abnahme kann sowohl der echte Drehgeber als auch der Pico gefordert sein – halte die Zuordnung **IN0/IN1** und eine stabile **Masseverbindung** immer ein.
