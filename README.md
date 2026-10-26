# Descent Engine

[![x86-64 Linux](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.x86-64.yml/badge.svg?branch=master)](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.x86-64.yml)
[![AArch64 Linux](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.aarch64.yml/badge.svg?branch=master)](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.aarch64.yml)

Descent Engine is an experiment in building a minimal game engine from the ground up in C++17, targeting x86-64 and AArch64 Linux exclusively.

The project prioritizes low-level systems programming, explicit control over implementation details, and minimal reliance on general-purpose abstractions. Rather than serving as a general-purpose engine, it provides a focused environment for exploring the construction of engine infrastructure close to the underlying platform.

## Design Principles

- **Minimalism:** Keep the engine focused and avoid unnecessary abstractions, dependencies, and general-purpose functionality.
- **Custom infrastructure:** Implement useful facilities directly within the project rather than relying on the C++ standard library. Implementations may favor the project's specific requirements over generality or conventional interfaces.
- **Explicit control:** Prefer transparent, deliberate control over memory, data representation, and runtime behavior.
- **Platform specificity:** Target x86-64 and AArch64 Linux.

## Technical Constraints

- **Language:** C++17.
- **Target platform:** x86-64 and AArch64 Linux.
- **C++ standard library:** Standard library headers are prohibited, except where their use is unavoidably necessary. Exceptions require documentation. Required functionality should generally be implemented within the project, potentially using specialized or idiosyncratic designs.
- **System interfaces:** Minimal use of the C standard library, POSIX APIs, and Linux-specific interfaces is permitted. C headers should be included within `extern "C"` blocks where appropriate.
- **Exceptions:** C++ exceptions are disabled.
- **Numeric types:** Prefer fixed-width numeric types defined in [`include/descent/type/core.hpp`](include/descent/type/core.hpp) where practical.
- **Third-party libraries:** Selected dependencies are permitted when they provide necessary platform integration or complex functionality that would be impractical to implement within the project.

## Graphics and Windowing

The engine targets Vulkan for graphics and Wayland for window-system integration. Additional libraries may be used where appropriate, particularly for implementing complex protocols, schemas, or other specialized interfaces.

## Building

### Requirements

| Requirement | Needed for | Ubuntu package |
|---|---|---|
| CMake 3.26 or later | Configuring | `cmake` |
| Ninja | The generator the presets use | `ninja-build` |
| clang and clang++ (21 is what CI uses) | The compiler the presets select | `clang` |
| `lld` and `llvm` (`llvm-ar`) | Release presets, which use LTO | `lld`, `llvm` |
| clang runtime libraries | The debug preset, which enables `-fsanitize=undefined` | `libclang-rt-21-dev` |
| clang-format 21 | The `check-format` target and `test-descent-format`; optional, skipped when absent | `clang-format-21` |
| pkg-config | Locating Wayland (client only) | `pkg-config` |
| Vulkan headers | The client only | `libvulkan-dev` |
| Wayland client library and headers | The client only | `libwayland-dev` |

The client is the only part of the project that needs Vulkan and Wayland. On Ubuntu:

```sh
sudo apt-get install cmake ninja-build clang lld llvm libclang-rt-21-dev clang-format-21 pkg-config libvulkan-dev libwayland-dev
```

### Configuring and building

```sh
cmake --preset debug -B build/debug
cmake --build build/debug
```

The presets are `debug`, `release`, `release-debug` and `release-size`. Builds must be out of source.

| Option | Default | Effect |
|---|---|---|
| `DESCENT_BUILD_CLIENT` | `ON` | Builds the client and the renderer library it uses. Turn it off to build everything else without Vulkan, Wayland or pkg-config. |
| `DESCENT_BUILD_TESTS` | `OFF` | Builds the tests in `test/` and registers them with CTest. |

For example, to build and test everything except the client:

```sh
cmake --preset debug -B build/debug -DDESCENT_BUILD_CLIENT=OFF -DDESCENT_BUILD_TESTS=ON
cmake --build build/debug
ctest --test-dir build/debug --output-on-failure
```

## Project Status

Descent Engine is an experimental project focused on exploring minimal engine architecture and low-level systems programming. Its design and implementation may evolve as development progresses.
