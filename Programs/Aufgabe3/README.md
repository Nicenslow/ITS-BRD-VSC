# Aufgabe 3 — BMP-Anzeige auf dem ITS-Board

Empfaengt `.bmp`-Dateien ueber USB-UART (Python GUI) und zeigt sie auf dem LCD an.

## Teilaufgaben

- **a)** Pixelweise Dekodierung und Ausgabe mit `GUI_drawPoint` (in `bmp.c` ueber `BMP_USE_WRITE_LINE 0`)
- **b)** Beschleunigte zeilenweise Ausgabe mit `GUI_WriteLine` (Standard: `BMP_USE_WRITE_LINE 1`)

Teilaufgabe c (Skalierung) ist nicht implementiert.

## Bedienung

1. Programm auf dem Board flashen und starten (Display zeigt kurz einen Hinweis)
2. Python GUI starten: `.venv\Scripts\python.exe main.py` in `GUI/Python_GUI`
3. **Load Files** → **Connect** → **Start** (Reihenfolge wichtig)
4. **S6** am Adapter (PG5) oder blauen **User-Taster** auf dem Nucleo druecken
5. Nach dem ersten Bild kein Hinweistext mehr — Display nur noch fuer Bilder

**Erster Test:** `480_320_pixel_ITSboard_8_bit_compressed.bmp` oder `22x14_8_bit_komprimiert_sehr_kleines_testbild.bmp`  
(Grosse Bilder wie 1280x854 muessen erst viele unsichtbare Zeilen dekodiert werden — der Bildschirm bleibt laenger schwarz.)

Hinweis: Ohne **Start** in der GUI wartet das Board nach dem Tastendruck auf Daten.  
Nach fehlgeschlagenem Transfer: Board resetten und GUI **Disconnect → Connect → Start**.

## Build (ITS-BRD_VSC)

```bash
cd Programs/Aufgabe3
cbuild Aufgabe3.csolution.yml --context Aufgabe3.Debug+ITSboard
```

## Quellcode fuer Kollegen

Eigene `.c`/`.h`-Dateien liegen in `Src/` und `Inc/` dieses Repos.
