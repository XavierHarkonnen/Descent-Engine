# Descent Engine

[![License](https://img.shields.io/badge/License-Apache--2.0-blue.svg)](./LICENSE)
[![x86-64 Linux](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.x86-64.yml/badge.svg?branch=master)](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.x86-64.yml)
[![AArch64 Linux](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.aarch64.yml/badge.svg?branch=master)](https://github.com/XavierHarkonnen/Descent-Engine/actions/workflows/ci.aarch64.yml)

Descent Engine is an experimental, minimal game engine written from the ground up for x86-64 and AArch64 Linux.

The project emphasizes low-level systems programming, explicit resource management, and a minimal dependency footprint. Its client uses Vulkan for rendering and Wayland for window-system integration.

## Building

### Requirements

| Requirement                        | Needed for                     | Ubuntu package       |
|------------------------------------|--------------------------------|----------------------|
| CMake 3.26 or later                | Configuration                  | `cmake`              |
| Ninja                              | Preset default generator       | `ninja-build`        |
| clang and clang++                  | Preset default compiler        | `clang`              |
| `lld` and `llvm` (`llvm-ar`)       | Release presets                | `lld`, `llvm`        |
| clang runtime libraries            | Debug preset                   | `libclang-rt-21-dev` |
| clang-format 21                    | `check-format` (optional)      | `clang-format-21`    |
| pkg-config                         | Locating Wayland (client only) | `pkg-config`         |
| Vulkan headers                     | Client                         | `libvulkan-dev`      |
| Wayland client library and headers | Client                         | `libwayland-dev`     |

The client is the only part of the project that needs Vulkan and Wayland. On Ubuntu:

```sh
sudo apt-get install cmake ninja-build clang lld llvm libclang-rt-21-dev clang-format-21 pkg-config libvulkan-dev libwayland-dev
```

### Configuring and building

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The presets are `debug`, `release`, `release-debug` and `release-size`. Each has a configure, build and test preset of the same name, builds in `build/<preset>`, and builds the tests. The test presets show the output of failing tests.

| Option                 | Default | Effect                                                                                                                           |
|------------------------|---------|----------------------------------------------------------------------------------------------------------------------------------|
| `DESCENT_BUILD_CLIENT` | `ON`    | Builds the client and the rendering library it uses. Turn it off to build everything else without Vulkan, Wayland or pkg-config. |
| `DESCENT_BUILD_TESTS`  | `OFF`   | Builds the tests in `test/` and registers them with CTest.                                                                       |

The presets turn `DESCENT_BUILD_TESTS` on. For example, to build and test everything except the client:

```sh
cmake --preset debug -DDESCENT_BUILD_CLIENT=OFF
cmake --build --preset debug
ctest --preset debug
```

## License

This project is licensed under the **Apache License 2.0** - see the [License](./LICENSE) for details.

Copyright © 2025 XavierHarkonnen9 and Enlarium

## Project Status

Descent Engine is an experimental project focused on exploring minimal engine architecture and low-level systems programming. Its design and implementation may evolve as development progresses.
