# Aufgabe 1 – RPN-Taschenrechner

RPN-Taschenrechner für das ITS-Board (WiSe 2026). Jede Praktikumsaufgabe liegt als eigenes CMSIS-Projekt unter `Programs/`, damit die Aufgaben getrennt gebaut und abgegeben werden können.

## Projekt öffnen

1. In VS Code das Workspace-Verzeichnis `ITS-BRD-VSC` öffnen.
2. Im CMSIS-View über `...` → `Select active Solution from Workspace` die Datei `Programs/Aufgabe1/Aufgabe1.csolution.yml` wählen.
3. Danach `...` → `Refresh (reload packs, update RTE)`.
4. Auf `vcpkg-configuration.json` im Projektordner: `Update Tool registry`, dann `Reactivate Environment`.
5. Im CMSIS-View mit dem Hammer bauen und mit `Load & Run` auf das Board laden.

Die Demo aus MS Teams kann vor dem eigenen Build an diese Stelle kopiert werden:

`Programs/Aufgabe1/out/Aufgabe1/ITSboard/Debug/Aufgabe1.axf`

## Module

| Modul | Dateien | Aufgabe |
| --- | --- | --- |
| token, scanner, display | bereitgestellte Eingabe und Ausgabe | Tastatur lesen, zwei Terminals zeichnen |
| stack | `stack.c` | Werte speichern, `d` und `r` |
| arithmetic | `arithmetic.c` | `+ - * /` mit Bereichsprüfung |
| format | `format.c` | `int` als Dezimaltext, auch negativ |
| calc_error | `calc_error.c` | Fehlermeldung auf dem Display |
| main | `main.c` | Schleife, `p`, `P` und `C` |

`calc_error` heißt absichtlich nicht `error`: `ITS_BRD_LIB` enthält bereits `error.h` für den LED-Error-Handler.

## Bedienung

Zahlen werden mit der Enter-Taste (unbeschriftete Taste) getrennt. Enter erreicht das Hauptprogramm nicht, der Scanner verarbeitet es.

`p` gibt den obersten Wert aus, ohne den Stack zu ändern. `P` gibt bis zu drei oberste Werte in einer Zeile aus, der oberste Wert steht am Ende. `C` löscht den Stack. Nach einem Fehler nimmt der Rechner nur noch `C` an, setzt sich zurück und löscht die Meldung.

## Unterlagen

- `Docs/Modulkonzept.md`
- `Docs/Fehlerkonzept.md`
- `Docs/Tests.md`

## Logiktest auf dem PC

Stack, Arithmetik und Zahlenformat hängen nicht am Board:

```text
gcc -std=c99 -Wall -Wextra -I Inc tests/test_logic.c Src/stack.c Src/arithmetic.c Src/format.c -o test_logic
```

Der Aufruf erfolgt im Ordner `Programs/Aufgabe1`.
