# Raytracer Project Documentation

This document provides comprehensive documentation for the raytracer project, including architecture, design decisions, and implementation details.

## Table of Contents

- [Project Overview](#project-overview)
- [Architecture](#architecture)
- [Core Components](#core-components)
- [Plugin System](#plugin-system)
- [Data Types](#data-types)
- [Build System](#build-system)
- [Scene Configuration](#scene-configuration)
- [Design Patterns](#design-patterns)
- [Memory Management](#memory-management)
- [CLI Interface](#cli-interface)
- [Future Improvements](#future-improvements)

---

## Project Overview

A CPU-based raytracer written in C++20 that generates realistic images by simulating the inverse path of light. The project emphasizes:

- **Extensibility**: Plugin-based architecture for shapes, materials, and lights
- **Modularity**: Clean separation of concerns with interfaces and factories
- **Performance**: Direct rendering without GUI overhead
- **Standards Compliance**: EPITECH project requirements (see PDF specification)

### Key Features

**Implemented (Must/Should):**
- Primitives: Sphere, Plane (Cylinder, Cone planned)
- Transformations: Translation (Rotation planned)
- Lighting: Point light, Ambient light, Drop shadows
- Materials: Lambertian (flat color with diffuse shading)
- Scene configuration: libconfig++ parser
- Interface: PPM file output

**Technical Requirements Met:**
- ✅ Interfaces for primitives and lights (C++ polymorphism)
- ✅ Plugin system (.so dynamic loading)
- ✅ Factory pattern for object creation
- ✅ No GUI, outputs to PPM
- ✅ CMake build system

---

## Architecture

### High-Level Design

```
┌─────────────────────────────────────────────────────────┐
│                    Main (Entry Point)                   │
│                     ./raytracer <scene.cfg>             │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                      Core                               │
│  - Orchestrates rendering                               │
│  - Manages Scene, Camera, Image                         │
│  - trace() for recursive ray tracing                    │
└───┬────────────────────────────────┬────────────────────┘
    │                                │
    ▼                                ▼
┌─────────────────────┐    ┌────────────────────────────┐
│   SceneParser       │    │      Image                 │
│  - Reads config     │    │  - Pixel buffer            │
│  - Uses Factories   │    │  - PPM writer              │
└──────┬──────────────┘    └────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────┐
│                    Factories                            │
│  ShapeFactory   MaterialFactory   LightFactory          │
│  - Load plugins dynamically via PluginLoader            │
│  - Create objects from config                           │
└──────┬──────────────────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────┐
│                  Plugin System                          │
│  plugins/shapes/sphere.so                               │
│  plugins/materials/lambertian.so                        │
│  plugins/lights/pointlight.so                           │
│  - Loaded at runtime via dlopen                         │
│  - Export extern "C" create() functions                 │
└─────────────────────────────────────────────────────────┘
```

### Directory Structure

```
raytracer/
├── src/
│   ├── main.cpp                    # Entry point
│   ├── core/
│   │   ├── Core.{hpp,cpp}          # Rendering orchestrator
│   │   ├── Image.{hpp,cpp}         # Pixel buffer & PPM output
│   │   └── PluginLoader.{hpp,cpp}  # Dynamic library loader
│   ├── Interfaces/
│   │   ├── IShape.hpp              # Shape interface (hit detection)
│   │   ├── IMaterial.hpp           # Material interface (shade, scatter)
│   │   └── ILight.hpp              # Light interface (illumination)
│   ├── DataTypes/
│   │   ├── Vec3.hpp                # 3D vector math (constexpr)
│   │   ├── Ray.hpp                 # Ray (origin + direction)
│   │   ├── HitRecord.hpp           # Ray-object intersection data
│   │   ├── Camera.hpp              # Camera (view frustum, ray generation)
│   │   ├── World.hpp               # Container for all shapes
│   │   └── Scene.hpp               # Complete scene (world + camera + lights)
│   ├── factories/
│   │   ├── ShapeFactory.{hpp,cpp}
│   │   ├── MaterialFactory.{hpp,cpp}
│   │   └── LightFactory.{hpp,cpp}
│   ├── parsers/
│   │   ├── SceneParser.{hpp,cpp}   # libconfig++ scene parser
│   └── plugins/
│       ├── Shapes/
│       │   └── Sphere.{hpp,cpp}
│       ├── Materials/
│       │   └── Lambertian.{hpp,cpp}
│       └── Lights/
│           └── PointLight.{hpp,cpp}
├── plugins/                        # Built .so files
│   ├── shapes/sphere.so
│   ├── materials/lambertian.so
│   └── lights/pointlight.so
├── scenes/                         # Example scene files (.cfg)
├── tests/                          # Unit tests (Criterion)
├── build/                          # CMake build directory
├── CMakeLists.txt
├── .clang-format                   # Code style
├── .clang-tidy                     # Static analysis
└── raytracer                       # Built executable
```

---

## Core Components

### 1. Core (`src/core/Core.{hpp,cpp}`)

**Purpose:** Orchestrates the entire rendering process.

**Key Responsibilities:**
- Load scene via `SceneParser`
- Generate rays for each pixel via `Camera`
- Trace rays recursively through the scene (`trace()`)
- Handle direct lighting (shadows, diffuse)
- Handle indirect lighting (reflections, max depth)
- Write final image to PPM

**Key Methods:**
```cpp
bool simulate();  // Main entry point: load scene, render, save
Vec3 trace(const Ray& ray, const Scene& scene, int depth);  // Recursive ray tracing
```

**Rendering Algorithm:**
1. For each pixel (x, y):
   - Generate ray from camera
   - Call `trace(ray, scene, maxDepth)`
2. `trace()` logic:
   - Check if ray hits any object in World
   - If miss: return background color
   - If hit:
     - Start with ambient lighting
     - For each light source:
       - Cast shadow ray
       - If not in shadow: add diffuse contribution
     - If material scatters (reflective):
       - Recursively trace scattered ray (depth - 1)
3. Write pixel color to Image

### 2. Image (`src/core/Image.{hpp,cpp}`)

**Purpose:** Manages pixel buffer and PPM file output.

**Key Responsibilities:**
- Store RGB pixel data
- Clamp colors to [0, 255]
- Write PPM P3 format (ASCII)

### 3. SceneParser (`src/parsers/SceneParser.{hpp,cpp}`)

**Purpose:** Parse scene configuration files using libconfig++.

**Key Methods:**
```cpp
Scene parse(const std::string& filename);
void parseCamera(libconfig::Config& config, Scene& scene);
void parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);
void parseShapes(libconfig::Config& config, const std::unordered_map<...>& materialMap, World& world);
void parseLights(libconfig::Config& config, std::vector<std::shared_ptr<ILight>>& lights, ...);
```

**Design Decision:** Materials are parsed first and stored in a map. Shapes then reference materials by name. This allows material reuse across multiple shapes.

---

## Plugin System

### Overview

The raytracer uses a dynamic plugin system that loads `.so` (shared object) files at runtime. This allows shapes, materials, and lights to be added without recompiling the main executable.

### Components

**1. PluginLoader** (`src/core/PluginLoader.{hpp,cpp}`)
- RAII wrapper around libdl (`dlopen`, `dlsym`, `dlclose`)
- Manages plugin lifecycle (loading, symbol resolution, unloading)
- Stores loaded plugin handles in an `unordered_map<string, void*>`

**2. Factories** (`src/factories/`)
- `ShapeFactory` - Loads and creates shape plugins
- `MaterialFactory` - Loads and creates material plugins
- `LightFactory` - Loads and creates light plugins
- Each factory maintains its own PluginLoader instance and function pointer cache

**3. Plugins** (`src/plugins/`)
- Located in `plugins/{shapes,materials,lights}/`
- Each plugin exports an `extern "C" create()` function
- Built as separate shared libraries (.so files)

### Plugin Contract

Each plugin must export a factory function with C linkage:

```cpp
extern "C" IInterface* create(<plugin-specific-parameters>);
```

Where:
- `IInterface` is `IShape`, `IMaterial`, or `ILight`
- Parameters are plugin-specific (no void* or variadic args)
- Function name is always `"create"`
- Returns a raw pointer; caller takes ownership

#### Plugin Examples

**Shape plugin (sphere.so):**
```cpp
extern "C" IShape* create(double x, double y, double z, double radius, std::shared_ptr<IMaterial>* material) {
    return new Sphere(Vec3(x, y, z), radius, *material);
}
```

**Material plugin (lambertian.so):**
```cpp
extern "C" IMaterial* create(double r, double g, double b) {
    return new Lambertian(Vec3(r, g, b));
}
```

**Light plugin (pointlight.so):**
```cpp
extern "C" ILight* create(double x, double y, double z, double r, double g, double b) {
    return new PointLight(Vec3(x, y, z), Vec3(r, g, b));
}
```

### Plugin Memory Management

**Key Decision:** Pass `std::shared_ptr<IMaterial>*` across plugin boundaries.

**Why:** Materials are shared between multiple shapes (stored in `SceneParser::materialMap`). Passing by pointer allows proper shared ownership without double-delete issues.

**Pattern:**
```cpp
// In Factory
std::shared_ptr<IMaterial> material = /* from materialMap */;
auto createFunc = reinterpret_cast<IShape*(*)(...)>(_createFunctions["sphere"]);
return std::shared_ptr<IShape>(createFunc(x, y, z, radius, &material));

// In Plugin
extern "C" IShape* create(..., std::shared_ptr<IMaterial>* material) {
    return new Sphere(..., *material);  // Copies shared_ptr, increments refcount
}
```

### Adding New Plugins

1. **Create plugin source files:**
   ```
   src/plugins/Shapes/NewShape.cpp
   src/plugins/Shapes/NewShape.hpp
   ```

2. **Implement the interface:**
   ```cpp
   class NewShape : public IShape {
       bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;
   };

   extern "C" IShape* create(...params...) {
       return new NewShape(...);
   }
   ```

3. **Add to CMakeLists.txt:**
   ```cmake
   add_raytracer_plugin(newshape shapes src/plugins/Shapes/NewShape.cpp)

   add_custom_target(shapes
       DEPENDS sphere newshape  # Add here
   )
   ```

4. **Add factory handler:**
   ```cpp
   // In ShapeFactory.cpp
   static std::unordered_map<std::string, ShapeCreator> creators = {
       {"sphere", _createSphere},
       {"newshape", _createNewShape},  // Add here
   };

   std::shared_ptr<IShape> ShapeFactory::_createNewShape(const libconfig::Setting& config,
                                                          std::shared_ptr<IMaterial> material) {
       // Extract config, cast function pointer, call plugin
   }
   ```

---

## Data Types

### Vec3 (`src/DataTypes/Vec3.hpp`)

**Purpose:** 3D vector for positions, directions, and colors.

**Design Decisions:**
- `constexpr` methods for compile-time evaluation
- Operator overloading for intuitive math (`+`, `-`, `*`, `/`, `dot`, `cross`)
- Non-member utility functions (`length`, `normalize`, `dot`, `cross`)

**Usage:**
```cpp
Vec3 position(0, 0, 0);
Vec3 direction = normalize(Vec3(1, 1, 1));
double dist = length(position - direction);
Vec3 result = cross(Vec3(1,0,0), Vec3(0,1,0));  // (0, 0, 1)
```

### Ray (`src/DataTypes/Ray.hpp`)

**Purpose:** Represents a ray (origin + direction).

```cpp
class Ray {
    Vec3 origin() const;
    Vec3 direction() const;
    Vec3 at(double t) const;  // Returns origin + t * direction
};
```

### HitRecord (`src/DataTypes/HitRecord.hpp`)

**Purpose:** Stores ray-object intersection data.

**Fields:**
- `Vec3 point` - Hit location in world space
- `Vec3 normal` - Surface normal at hit point
- `double t` - Ray parameter (distance along ray)
- `bool front_face` - Whether ray hit from outside
- `std::shared_ptr<IMaterial> material` - Material at hit point

**Key Method:**
```cpp
void set_face_normal(const Ray& r, const Vec3& outward_normal);
```
Determines if ray hits from inside or outside, flips normal accordingly.

### Camera (`src/DataTypes/Camera.hpp`)

**Purpose:** Generates rays for each pixel based on view parameters.

**Parameters:**
- Resolution (width, height)
- Position (eye point)
- Look-at point
- Up vector
- Field of view (FOV)

**Key Method:**
```cpp
Ray getRay(float u, float v) const;  // u, v in [0, 1]
```

### World (`src/DataTypes/World.hpp`)

**Purpose:** Container for all shapes in the scene.

**Key Method:**
```cpp
bool get_closest_hit(const Ray& ray, double t_min, double t_max, HitRecord& hit) const;
```
Tests ray against all objects, returns closest intersection.

### Scene (`src/DataTypes/Scene.hpp`)

**Purpose:** Complete scene description.

**Contains:**
- `World` (all shapes)
- `Camera`
- `std::vector<std::shared_ptr<ILight>>` (all lights)
- Ambient/diffuse multipliers

---

## Build System

### CMake Structure

**Main CMakeLists.txt:**
- C++20 standard
- Compiler flags: `-Wall -Wextra -Werror -g3`
- libconfig++ dependency (pkg-config or manual find)
- Criterion for tests (FetchContent fallback)
- Plugin helper function: `add_raytracer_plugin()`

**Plugin Build:**
```cmake
add_raytracer_plugin(sphere shapes src/plugins/Shapes/Sphere.cpp)
```

This creates a shared library with:
- No 'lib' prefix (outputs `sphere.so`, not `libsphere.so`)
- Correct output directory (`./plugins/shapes/`)
- Access to project include directories

### Build Targets

**From build directory:**
```bash
make                  # Build everything
make raytracer        # Build main executable only
make sphere           # Build sphere plugin
make shapes           # Build all shape plugins
make materials        # Build all material plugins
make lights           # Build all light plugins
make plugins          # Build all plugins
```

**From project root:**
```bash
cmake --build build --target raytracer
cmake --build build --target plugins
# or
make -C build
make -C build plugins
```

### Build Workflow

1. **Configure:**
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   ```

2. **Build:**
   ```bash
   cmake --build build
   ```

3. **Run:**
   ```bash
   ./raytracer scenes/example.cfg
   ```

---

## Scene Configuration

### libconfig++ Format

Scene files use libconfig++ syntax (similar to JSON/INI).

**Example Structure:**
```
# Camera configuration
camera:
{
    resolution = { width = 1920; height = 1080; };
    position = { x = 0; y = -100; z = 20; };
    look_at = { x = 0; y = 0; z = 0; };
    up = { x = 0; y = 0; z = 1; };
    fieldOfView = 72.0;
};

# Materials (define once, reference by name)
materials:
{
    lambertian = (
        { name = "red"; color = { r = 255; g = 64; b = 64; }; },
        { name = "green"; color = { r = 64; g = 255; b = 64; }; }
    );
};

# Shapes reference materials
shapes:
{
    spheres = (
        {
            position = { x = 60; y = 5; z = 40; };
            radius = 25;
            material = "red";
        }
    );
};

# Lights
lights:
{
    ambient = 0.4;
    diffuse = 0.6;

    point = (
        {
            position = { x = 400; y = 100; z = 500; };
            color = { r = 255; g = 255; b = 255; };
            intensity = 1.0;
        }
    );
};
```

### Parser Flow

1. **Parse materials** → store in `std::unordered_map<string, shared_ptr<IMaterial>>`
2. **Parse shapes** → look up materials by name, create shapes
3. **Parse lights** → create light objects
4. **Parse camera** → set up view

---

## Design Patterns

### 1. Factory Pattern

**Used in:** ShapeFactory, MaterialFactory, LightFactory

**Purpose:** Encapsulate object creation, allow runtime type selection.

**Implementation:**
- Static `create()` method takes type name and config
- Internal dispatch table maps type names to creator functions
- Each creator function loads plugin and calls its `create()` function

### 2. Strategy Pattern (via Interfaces)

**Used in:** IShape, IMaterial, ILight

**Purpose:** Define family of algorithms (hit detection, shading, illumination) and make them interchangeable.

**Example:**
```cpp
class IShape {
    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const = 0;
};

class Sphere : public IShape {
    bool hit(...) const override { /* sphere intersection math */ }
};
```

### 3. Plugin Pattern

**Used in:** Entire plugin system

**Purpose:** Allow functionality to be added at runtime without recompilation.

**Implementation:** Dynamic library loading with extern "C" factory functions.

### 4. RAII (Resource Acquisition Is Initialization)

**Used in:** PluginLoader

**Purpose:** Automatic resource management (plugin handles).

**Implementation:**
- Constructor/`load()` calls `dlopen`
- Destructor calls `dlclose` on all handles
- No manual cleanup needed

---

## Memory Management

### Smart Pointers

**Used throughout:**
- `std::shared_ptr<IShape>` - Shapes stored in World
- `std::shared_ptr<IMaterial>` - Materials shared between shapes
- `std::shared_ptr<ILight>` - Lights stored in Scene

**Why shared_ptr?**
- Automatic reference counting
- Safe shared ownership (materials used by multiple shapes)
- No manual delete needed

### Ownership Model

**Scene owns everything:**
```
Core
 └─ Scene
     ├─ World (owns shapes via shared_ptr)
     ├─ Camera (value type)
     └─ Lights (owns lights via shared_ptr)
```

**Materials are shared:**
- SceneParser creates materialMap
- Multiple shapes reference same material via shared_ptr
- Material deleted when last shape is destroyed

### Plugin Boundary

**Raw pointers cross plugin boundary:**
- Plugins return raw `IInterface*`
- Factories wrap in `shared_ptr` immediately
- Core manages lifetime, plugins don't delete

**Exception:** `shared_ptr<IMaterial>*` passed to shape plugins
- Allows plugin to copy shared_ptr (increment refcount)
- Avoids double-delete issue

---

## Design Decisions

### Why not IPlugin interface?

The config file already separates plugin types by section (shapes, lights, materials), so the parser knows which factory to call. A unified plugin registry would add complexity without benefit.

### Why not void* parameters?

Type-safety. Each factory knows what plugin it's loading and can cast the function pointer to the correct signature. This catches errors at compile-time instead of runtime.

### Why not variadic arguments (va_args)?

Even less type-safe than void*. Easy to crash with wrong arg count/types. Harder to debug.

### Why pass shared_ptr by pointer?

Because materials are shared resources stored in a map. Multiple shapes reference the same material instance. Passing `shared_ptr*` allows the plugin to copy the shared_ptr, properly incrementing the reference count without double-delete.

### Why extern "C"?

Prevents C++ name mangling, ensuring the symbol "create" can be found via `dlsym()` regardless of compiler.

### Why libconfig++?

- Readable syntax (better than raw JSON for humans)
- Type-safe access to config values
- Supports comments in config files
- Specified in project requirements

### Why PPM output?

- Dead simple format (ASCII text)
- No external image library needed
- Easy to convert to PNG/JPG with external tools
- Specified in project requirements

---

## CLI Interface

### Help Function

**Location:** `src/main.cpp` (main.cpp:4-15)

**Purpose:** Display available shapes, materials, and lights to help users understand what features are currently implemented in the raytracer.

**Invocation:**
```bash
./raytracer --help
# or
./raytracer
```

**Current Implementation:**
The `help()` function lists:
- **Shapes:** Sphere, Cylinder (with their parameters)
- **Materials:** Lambertian, Transparent (with their parameters)
- **Lights:** PointLight (with its parameters)

**Design Decision:** Whenever new plugins are added (shapes, materials, lights), the help function should be updated to reflect the new capabilities. This provides users with a quick reference without needing to read documentation or source code.

**Maintenance:** Remember to update the help function when adding new plugins to keep it in sync with actual capabilities.

---

## Future Improvements

### Rendering Features

- [ ] Additional primitives: Plane, Cylinder, Cone, Torus
- [ ] Transformation matrix support (scale, rotation, shear)
- [ ] Additional materials: Reflective, Refractive, Textured
- [ ] Phong reflection model (specular highlights)
- [ ] Ambient occlusion (soft shadows)
- [ ] Antialiasing (supersampling, adaptive)
- [ ] Normal mapping
- [ ] Multi-sampling for smoother edges

### Performance

- [ ] Multithreading (parallel pixel rendering)
- [ ] Space partitioning (BVH, octree)
- [ ] SIMD vectorization for Vec3 operations
- [ ] GPU acceleration (CUDA/OpenCL)
- [ ] Clustering (distributed rendering)

### Plugin System

- [ ] Error reporting: Add `dlerror()` messages
- [ ] Plugin metadata: Version, author, description
- [ ] Hot reloading: Unload/reload plugins at runtime
- [ ] Plugin validation: Check ABI compatibility
- [ ] Plugin discovery: Auto-scan plugins directory

### Tooling

- [ ] GUI for real-time preview (SFML)
- [ ] Scene editor
- [ ] Automatic scene reload on file change
- [ ] Progress bar during rendering
- [ ] Render time statistics

---

## References

- **Project PDF**: `G-OOP-400_raytracer.pdf` - Complete specification
- **Issue #9**: "Make Core load plugins" - Plugin system implementation
- **libconfig++**: https://hyperrealm.github.io/libconfig/
- **CMake**: https://cmake.org/cmake/help/latest/

---

## Quick Start

```bash
# Build everything
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run with a scene
./raytracer scenes/example.cfg

# Output will be saved as output.ppm
# Convert to PNG (requires ImageMagick)
convert output.ppm output.png
```

---

*This documentation was generated with assistance from Claude Code.*
