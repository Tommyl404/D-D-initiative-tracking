# Build and Deployment Notes

## Prerequisites

- CMake 3.16+
- A C++17 compiler
- Qt 6 (Widgets, Gui, Core, Test). Qt 5.15+ is also supported.

## Configure and Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Deploying

#### Windows

Use `windeployqt` with the built executable:

```powershell
windeployqt --release path\to\dnd_initiative.exe
```

#### macOS

Use `macdeployqt` to bundle frameworks:

```bash
macdeployqt dnd_initiative.app
```

#### Linux

Consider packaging as an AppImage using [linuxdeployqt](https://github.com/probonopd/linuxdeployqt).

