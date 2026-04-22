# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Course project (курсовий проєкт) — a Windows console simulator for comparing network routing algorithms. UI and output strings are in Ukrainian. Single-project Visual Studio 2022 solution (`kursova.sln`), C++ with MSVC toolset `v143`, `/EHsc`, Unicode character set.

## Build & Run

Two supported build paths:

- **MSBuild / Visual Studio (authoritative):**
  - `msbuild kursova.sln /p:Configuration=Debug /p:Platform=x64`
  - Configurations: `Debug|x64`, `Release|x64`, `Debug|Win32`, `Release|Win32`. `x64` is what gets used — `.vs/` and prebuilt `kursova/kursova.exe` reflect that.
- **VS Code task (`.vscode/tasks.json`):** `cl.exe /Zi /EHsc /nologo ...` on the *active file*. Works here only because the project is a single `.cpp` with header-only deps; do not add new `.cpp` files without updating this task and `kursova.vcxproj`.

Run the built binary: `./kursova/kursova.exe` (the menu is interactive — arrow keys + Enter).

There is no test suite, no linter config, and no package manager. Do not invent commands for these.

## Architecture

Three files, all under `kursova/`:

- `kursova.cpp` — entry point + console UI. `main()` runs an arrow-key menu (`_getch` + VK codes `72`/`80`/`13`). The `Play` action (`showPlay`) **hardcodes a 5-node, 6-edge topology** and runs Dijkstra from node 1 → node 5. Changing the demo scenario means editing `showPlay()` directly. Uses `SetConsoleCP(1251)` / `SetConsoleOutputCP(1251)` for Cyrillic output and Win32 `SetConsoleTextAttribute` for colors.
- `NetworkGraph.h` — header-only `NetworkGraph` class. Undirected weighted graph as `unordered_map<int, vector<Edge>>` adjacency list keyed by node id; `addEdge` has an `isDirected=false` default that inserts both directions. `Node` carries `x,y` coords that nothing currently renders.
- `RoutingAlgorithms.h` — header-only `RoutingAlgorithms` class with a single static `dijkstra(graph, startId, targetId)` returning `RoutingResult { path, totalCost, iterations }`. The class name implies multiple algorithms are planned; only Dijkstra exists today. New algorithms should follow the same static-method + `RoutingResult` signature so `showPlay` can call them uniformly.

Everything is header-only aside from `kursova.cpp`, so rebuilds are whole-TU. Both `NetworkGraph.h` and `RoutingAlgorithms.h` do `using namespace std;` at file scope — be aware when adding new headers.

## Platform gotchas

- Windows-only: pulls in `<conio.h>` and `<windows.h>`; `system("cls")` for clears. Don't try to make it portable unless asked.
- Console code page is **CP1251**. Source files with Ukrainian string literals must be saved in a CP1251-compatible encoding or the console will print mojibake. `NetworkGraph::printGraph()` currently shows garbled output for this reason — the literals there are not CP1251-encoded. If editing Ukrainian strings, verify the file encoding matches what `SetConsoleOutputCP(1251)` expects, or the text will render as `������`.
- Build artifacts (`*.obj`, `*.ilk`, `*.pdb`, `*.exe`, `x64/`, `.vs/`) are checked in alongside source. There is no `.gitignore`. Don't delete them unless asked.
- `RoutingAlgorithms.h` has a duplicate `#pragma once` — harmless, leave it unless doing unrelated cleanup.
