# D&D Initiative Tracker

This repository provides a portable C++17 implementation of a tabletop initiative tracker focused on core combat logic, roster management, and JSON persistence. The project now targets a console-based workflow so that the underlying rules engine and storage helpers can be compiled and tested without a Qt toolchain.

## Building

```bash
cmake -S . -B build
cmake --build build
```

Run the unit tests with:

```bash
cd build
ctest
```

You can also execute the sample console application:

```bash
./dnd_initiative
```

## Project Layout

- `src/` – C++ sources for the core turn tracker, JSON stores, dice roller, and console entry point
- `tests/` – Lightweight unit tests that exercise initiative sorting, condition ticking, death saves, roster naming, and persistence round-tripping
- `docs/` – Additional documentation including build and schema details
- `resources/` – Placeholder assets for future front-ends
- `data/` – Sample JSON files for encounters, characters, and groups
