# D&D Initiative Tracker

This repository contains a Qt Widgets-based initiative tracker for tabletop role-playing games. It includes a basic GUI prototype, domain model, JSON persistence helpers, and unit tests that exercise critical initiative-management rules.

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

Or execute the test binary directly:

```bash
./testsuite
```

## Project Layout

- `src/` – C++ sources for the application
- `tests/` – Qt Test-based unit tests
- `docs/` – Additional documentation
- `resources/` – Icons and themes (placeholders)
- `data/` – Sample JSON files

