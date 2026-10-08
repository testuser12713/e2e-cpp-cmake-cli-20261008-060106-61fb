# csvstats

`csvstats` ist ein C++20-Kommandozeilenprogramm, das eine CSV-Datei einliest,
ihre numerischen Spalten erkennt und je Spalte Anzahl, Summe, Mittelwert,
Minimum und Maximum ausgibt. Das Trennzeichen lässt sich über `--delimiter`
setzen, eine einzelne Spalte über `--column` auswählen. Eine fehlende,
unlesbare oder leere Datei sowie eine unbekannte bzw. nicht numerische Spalte
führen zu einer klaren Meldung auf `stderr` und einem definierten Exit-Code.
Die gesamte Parse- und Rechenlogik liegt in der CLI-freien Bibliothek
`csvstats_core` und ist über CTest direkt testbar. Außer der
C++-Standardbibliothek wird nichts verwendet.

Dieses Repository enthält das Gerüst (Build, Bibliothek, CLI-Dispatch und
Tests). Die eigentliche Parser- und Statistiklogik folgt in eigenen Tickets.

## Tech-Stack

- Sprache: C++20
- Build: CMake (>= 3.20)
- Tests: CTest mit Unit-Tests ohne Dritt-Framework
- Abhängigkeiten: ausschließlich die C++-Standardbibliothek
- Warnungen: `-Wall -Wextra -Wpedantic`

## Installation / Voraussetzungen

Benötigt werden CMake >= 3.20 und ein C++20-fähiger Compiler (z. B. GCC 11+,
Clang 14+). Es müssen keine Pakete installiert werden.

## Build (Produktion)

```sh
cmake -S . -B build
cmake --build build
```

Das erzeugt die statische Bibliothek `csvstats_core`, die Testprogramme und die
ausführbare Datei `build/csvstats`.

## Tests

```sh
ctest --test-dir build --output-on-failure
```

Die Test-Suite deckt die Bibliothek (`csv`, `stats`, `report`), die
Argumentverarbeitung (`cli`) und einen Smoke-Test der echten Binärdatei
(`cli_smoke`) ab.

## Ausführen (Entwicklung)

Der Startbefehl entspricht `start` in `RUN.json`:

```sh
./build/csvstats --help
```

Die normale Verwendung liest eine CSV-Datei:

```sh
./build/csvstats daten.csv
./build/csvstats --delimiter ';' --column preis daten.csv
```

Wird `csvstats` ohne Argumente aufgerufen, erscheint die Usage-Zeile auf
`stderr` und das Programm endet mit Exit-Code 2.

## Verwendung

```
Usage: csvstats [OPTIONS] <file>
```

Optionen (werden im CLI-Ticket implementiert):

| Option | Bedeutung |
| --- | --- |
| `--delimiter X` | Trennzeichen setzen (Standard: `,`, `\t` für Tabulator) |
| `--column NAME` | nur diese Spalte ausgeben |
| `-h`, `--help` | Usage auf `stdout` ausgeben und mit 0 beenden |

## Exit-Codes

| Code | Bedeutung |
| --- | --- |
| 0 | Bericht ausgegeben oder keine numerische Spalte (Hinweis auf `stderr`) |
| 1 | Unbekannte Option oder fehlender Optionswert (Meldung + Usage auf `stderr`) |
| 2 | Fehlender Pfad, Datei nicht vorhanden oder nicht lesbar |
| 3 | Leere Datei oder Datei ohne Datenzeilen |
| 4 | `--column` unbekannt oder Spalte nicht numerisch |

## Funktionsumfang

- CLI-freie Kernbibliothek `csvstats_core` mit CSV-Parser, Statistik und
  Report-Formatierung (öffentliche Header unter `include/csvstats/`).
- Vollständiger Argument-Dispatch in `run()` inklusive Exit-Code-Mapping,
  Bericht auf `stdout` und Diagnosen auf `stderr`.
- CMake-Build mit `-Wall -Wextra -Wpedantic` und CTest-Registrierung von
  `csv`-, `stats`- und `report`-Randfällen sowie einem CLI-Smoke-Test.
