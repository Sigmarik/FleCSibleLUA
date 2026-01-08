# <div align="center">FleCSibleLua</div>

<div align="center">

**A Superset of Lua for Expressive Entity-Component-System Scripting**

[![Lua](https://img.shields.io/badge/Lua-2C2D72?style=for-the-badge&logo=lua&logoColor=white)](https://lua.org)
[![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org)
[![ECS](https://img.shields.io/badge/Flecs-43ba85?style=for-the-badge&logoColor=white)](https://www.flecs.dev)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](LICENSE)

</div>

---

## 📖 Overview

**FleCSibleLua (FLua)** is a superset of the Lua programming language designed specifically to simplify and streamline
Entity-Component-System (ECS) development with Flecs. By extending Lua's syntax with ECS-first constructs,
it enables game developers, simulation engineers, and researchers to write clear and maintainable
systems with minimal boilerplate.

> **Academic Note:** This project was developed as part of a bachelor's thesis in Computer Science, exploring the intersection of domain-specific languages and game architecture patterns.

---

## ✨ Key Features

- **🔧 ECS-Native Syntax** – Convenient constructs for defining systems and querying components
- **🚀 Zero-Boilerplate Systems** – Write game logic directly, with little to no setup
- **🔌 Seamless Integration** – Easy embedding into C++ Flecs projects
- **📚 Lua Compatibility** – Adapt your existing Lua scripts with minimal effort

---

## 🎮 Quick Examples

### Basic System
```lua
system(entity(Position, Velocity))
    entity.Position.x += entity.Velocity.x * deltaTime()
    entity.Position.y += entity.Velocity.y * deltaTime()
end
```

### Queries
```lua
function countMovingEntities()
    local count = 0
    
    for entity(Velocity) do
        local vel = entity.Velocity
        if (length(vel.x, vel.y) > 0.1) then
            count += 1
        end
    end
    
    return count
end
```

### Systems With Multiple Entities
```lua
system(alpha(Position, Velocity, BoxCollider, Mass),
       beta(Position, Velocity, BoxCollider, Mass))

    if (alpha == beta) then continue end
    
    if (!colliding(alpha, beta)) then continue end
    
    resolveCollision(alpha, beta)
end
```

---

## 🚀 Integration

### C++ Loading Example

Load and deploy your FleCSibleLua scripts with a few lines of code.

```cpp
#include <flecsible_lua.h>

// Load and configure FleCSibleLua scripts
flua::Script script = flua::Script::Load("scripts/main.lua");
script.overrideGlobal("SCREEN_X", window.getSize().x);
script.overrideGlobal("SCREEN_Y", window.getSize().y);

// Deploy to your ECS world
flua::DeployedScript deployment = script.deploy(world);

// Systems automatically integrate with your game loop
```

Extend your game logic with custom C++ functions that seamlessly integrate
with FleCSibleLua systems.

```cpp
#include <flecsible_lua_api.h>

void increment(FluaState* state)
{
    // Validate argument type (Lua stack position 0 = first argument)
    if (!state->isNumber(0))
        throw Error("Attempt to increment a non-numeric value");

    // Retrieve, increment, and push result back to Lua stack
    double value = state->getNumber(0);
    state->pushValue(value + 1.0);
}

// Later, during script initialization:
script.overrideGlobal("increment", increment);  // Expose to Lua as global function
```

### Build Integration

Tell FleCSibleLua which headers define ECS components.
No macros / lists needed.

<details>
<summary><b>CMake Configuration</b></summary>

You will need three files to integrate FLua into your project:
- **flua.cmake** can be found in the [examples/cmake](examples/cmake) directory
- **FluaComponentParser.exe** and **FlecsibleLua.lib** can be built from project's sources (instructions below)

```cmake
# Include flua.cmake to have access to `target_flua_components` 
include(flua.cmake)

# Tell FLua where your components are defined
target_flua_components(YourProject ${PATH_TO_PROJECT_TOOLS}/FluaComponentParser.exe
        components1.h components2.h ...)

target_link_libraries(YourProject PRIVATE ${PATH_TO_PROJECT_LIBRARIES}/FlecsibleLua.lib)
```

</details>

---

## 📦 Installation

### Building from Source

Currently, FleCSibleLua is available as a source-only distribution. To integrate it into your project, follow these steps:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Sigmarik/FleCSibleLUA.git
   cd FleCSibleLUA
   ```

2. **Build with CMake**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --target FlecsibleLua FluaComponentParser
   ```
   This will compile the runtime library (`FlecsibleLua.lib`) and the component parser (`FluaComponentParser.exe`).

3. **Copy to your project**:  
   Copy the following files to your project's dependencies directory:
    - `build/archive/[Debug|Release]/FleCSibleLua.lib` (static library)
    - `build/runtime/[Debug|Release]/FluaComponentParser.exe` (component parser executable)
    - The [`include/`](include) directory (for C++ headers)
    - [`cmake/flua.cmake`](cmake) (`target_flua_components` implementation for CMake projects)

4. **Integrate with your build system**:
   Add the include directory to your compiler's include path and link against `FleCSibleLua.lib`.\
   Ensure the `.cpp` file generated by `FluaComponentParser.exe` from ECS component headers is included into the project
   or use `target_flua_components` function defined in `flua.cmake`.
   ```bash
   FluaComponentParser.exe component_info_output.cpp components1.h components2.h ...
   ```

> **Platform Note**: Currently, FleCSibleLua is only supported on **Windows**. A Linux release is planned for future versions.

---

## 📚 Documentation

| Section                | Status      | Description                                                |
|------------------------|-------------|------------------------------------------------------------|
| **Language Reference** | *Planned*   | Complete FLua-specific syntax specification                |
| **API Documentation**  | *Planned*   | C++ integration guide                                      |
| **Tutorial Series**    | *Planned*   | From basics to advanced systems                            |
| **Example Projects**   | *Completed* | There are two example projects available [here](examples/) |

*Documentation will be hosted at [FleCSibleLUA/wiki](https://github.com/Sigmarik/FleCSibleLUA/wiki) upon release.*

---

## 🏗️ Project Structure

```
FleCSibleLUA/
├── cmake                  # CMake integration helpers
├── src
│   ├── component_parser   # Component parser sources
│   └── lib                # C++ integration library
├── examples               # Example games and simulations
├── external               # External dependencies
├── include                # Library headers
└── benchmarks             # Performance comparison tests
```

---

## 📝 TODO

- [ ] Implement Additional Data Types
  - [ ] Vectors
  - [ ] Integers _(for more precise calculations)_
- [ ] Expand Scripting Library
  - [ ] Strings
  - [ ] Math _(with vector operations)_
  - [ ] Tables
  - [ ] ECS
- [ ] Performance Tests
- [ ] Documentation
  - [ ] Language Reference
  - [ ] API Documentation

---

## 📄 License

FleCSibleLua is released under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgements

This project represents the culmination of my bachelor's thesis research at **Moscow Institute of Physics and Technology**. I would like to express my sincere gratitude to:

- [**Aleksei Lesovoi**](https://www.linkedin.com/in/aleksei-lesovoi/en), my research supervisor and professor, for his invaluable guidance, insightful feedback, and unwavering support throughout this research journey.
- The **Lua community** for maintaining an elegant and embeddable language.
- The **Flecs community** for building such an ingenious ECS.
- [**flecs-lua**](https://github.com/flecs-hub/flecs-lua) project for being an inspiration for this work.

---

<div align="center">

### 🚀 Ready to transform your Flecs development?

*Star the repository to stay updated on the release!*

</div>