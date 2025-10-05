# Build and Deployment Notes

## Prerequisites

- CMake 3.16+
- A C++17 compiler

The project is self-contained and does not rely on Qt or third-party package managers. All JSON parsing, persistence helpers, and tests are implemented in the repository.

## Configure and Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Running

From the build directory run the console prototype:

```bash
./dnd_initiative
```

The executable demonstrates loading the sample encounter, advancing turns, generating mass-added roster entries, and rolling initiative with advantage.
