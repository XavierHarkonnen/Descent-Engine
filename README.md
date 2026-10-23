# Descent Engine

Descent Engine is an experiment in building a minimal game engine from the ground up in C++17, targeting x86-64 Linux exclusively.

The project prioritizes low-level systems programming, explicit control over implementation details, and minimal reliance on general-purpose abstractions. Rather than serving as a general-purpose engine, it provides a focused environment for exploring the construction of engine infrastructure close to the underlying platform.

## Design Principles

- **Minimalism:** Keep the engine focused and avoid unnecessary abstractions, dependencies, and general-purpose functionality.
- **Custom infrastructure:** Implement useful facilities directly within the project rather than relying on the C++ standard library. Implementations may favor the project's specific requirements over generality or conventional interfaces.
- **Explicit control:** Prefer transparent, deliberate control over memory, data representation, and runtime behavior.
- **Platform specificity:** Target x86-64 Linux without requiring cross-platform compatibility.

## Technical Constraints

- **Language:** C++17.
- **Target platform:** x86-64 Linux.
- **C++ standard library:** Standard library headers are prohibited, except where their use is unavoidably necessary. Exceptions require documentation. Required functionality should generally be implemented within the project, potentially using specialized or idiosyncratic designs.
- **System interfaces:** Minimal use of the C standard library, POSIX APIs, and Linux-specific interfaces is permitted. C headers should be included within `extern "C"` blocks where appropriate.
- **Exceptions:** C++ exceptions are disabled.
- **Numeric types:** Prefer fixed-width numeric types defined in [`include/descent/type/core.hpp`](include/descent/type/core.hpp) where practical.
- **Third-party libraries:** Selected dependencies are permitted when they provide necessary platform integration or complex functionality that would be impractical to implement within the project.

## Graphics and Windowing

The engine targets Vulkan for graphics and Wayland for window-system integration. Additional libraries may be used where appropriate, particularly for implementing complex protocols, schemas, or other specialized interfaces.

## Project Status

Descent Engine is an experimental project focused on exploring minimal engine architecture and low-level systems programming. Its design and implementation may evolve as development progresses.