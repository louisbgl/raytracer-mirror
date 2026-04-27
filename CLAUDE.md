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

**Implemented:**
- **Primitives (13 shapes):** Sphere, Plane, Cylinder, Cone, Box, Rectangle, Triangle, Torus, Tanglecube, Hourglass, LimitedCylinder, LimitedCone, LimitedHourglass
- **Transformations:** Full rotation (Euler angles) and translation for bounded shapes via AShape
- **Lighting:** PointLight, DirectionalLight, ambient lighting, drop shadows
- **Materials (8 types):** Lambertian, ColoredDiffuse, Phong, Reflective, Refractive, Chessboard, PerlinNoise, ImageTexture
- **Advanced Rendering:** Antialiasing (SSAA), background images, UV texture coordinates
- **Scene Configuration:** libconfig++ parser with renderer settings
- **Optimization:** AABB-based ray culling, quadratic/cubic/quartic solvers
- **Interface:** PPM file output with configurable paths
- **Developer Tools:** Logging system, progress bar, metadata-driven help

**Technical Requirements Met:**
- ✅ Interfaces for primitives and lights (C++ polymorphism)
- ✅ Plugin system (.so/.dylib dynamic loading with metadata)
- ✅ Factory pattern for object creation
- ✅ No GUI, outputs to PPM
- ✅ CMake build system with aggressive Release optimizations

---

## Architecture

### High-Level Design

```
┌─────────────────────────────────────────────────────────┐
│                    Main (Entry Point)                   │
│              ./raytracer <scene.cfg> [--log]            │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                      Core                               │
│  - Orchestrates rendering with antialiasing             │
│  - Manages Scene, Camera, Image, Logger                 │
│  - trace() for recursive ray tracing                    │
│  - ProgressBar for real-time feedback                   │
└───┬────────────────────────────────┬────────────────────┘
    │                                │
    ▼                                ▼
┌─────────────────────┐    ┌────────────────────────────┐
│   SceneParser       │    │      Image                 │
│  - Reads config     │    │  - Pixel buffer            │
│  - Uses Factories   │    │  - PPM writer/reader       │
│  - Parses renderer  │    │  - Texture sampling        │
│    settings         │    └────────────────────────────┘
└──────┬──────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────┐
│                 PluginManager (Singleton)               │
│  - Scans plugins/ directory at startup                  │
│  - Loads all .so/.dylib files                           │
│  - Caches create() and metadata() functions             │
│  - Provides plugin discovery by category                │
└──────┬──────────────────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────┐
│                    Factories (Static)                   │
│  ShapeFactory   MaterialFactory   LightFactory          │
│  - Query PluginManager for create functions             │
│  - Parse libconfig settings                             │
│  - Return shared_ptr<IInterface>                        │
└──────┬──────────────────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────┐
│                  Plugin System                          │
│  plugins/shapes/{sphere,box,torus,...}.so               │
│  plugins/materials/{lambertian,phong,reflective,...}.so │
│  plugins/lights/{pointlight,directionallight}.so        │
│  - Export extern "C" create() + metadata()              │
│  - Metadata contains: name, plural, help, category      │
└─────────────────────────────────────────────────────────┘
```

### Directory Structure

```
raytracer/
├── src/
│   ├── main.cpp                    # Entry point
│   ├── core/
│   │   ├── Core.{hpp,cpp}          # Rendering orchestrator
│   │   ├── Image.{hpp,cpp}         # Pixel buffer, PPM I/O, texture sampling
│   │   ├── PluginLoader.{hpp,cpp}  # Dynamic library loader (RAII)
│   │   ├── PluginManager.{hpp,cpp} # Singleton plugin registry
│   │   ├── AShape.{hpp,cpp}        # Abstract shape with transformation/AABB
│   │   ├── Logger.{hpp,cpp}        # Structured logging with timestamps
│   │   └── ProgressBar.{hpp,cpp}   # Real-time rendering progress
│   ├── Interfaces/
│   │   ├── IShape.hpp              # Shape interface (hit detection)
│   │   ├── IMaterial.hpp           # Material interface (shade, scatter)
│   │   └── ILight.hpp              # Light interface (illumination)
│   ├── DataTypes/
│   │   ├── Vec3.hpp                # 3D vector math (constexpr)
│   │   ├── Ray.hpp                 # Ray (origin + direction)
│   │   ├── HitRecord.hpp           # Ray-object intersection data (with UV)
│   │   ├── Camera.hpp              # Camera (view frustum, ray generation)
│   │   ├── World.hpp               # Container for all shapes
│   │   ├── Scene.hpp               # Complete scene (world + camera + lights)
│   │   └── RendererConfig.hpp      # Antialiasing, lighting, background settings
│   ├── Math/
│   │   ├── AABB.hpp                # Axis-aligned bounding box (slab method)
│   │   ├── QuadraticSolver.hpp     # Quadratic equation solver
│   │   ├── CubicSolver.hpp         # Cubic equation solver
│   │   ├── QuarticSolver.hpp       # Quartic equation solver
│   │   ├── Roots3And4.c            # Polynomial root finding
│   │   └── Constants.hpp           # Math constants (PI, etc.)
│   ├── factories/
│   │   ├── ShapeFactory.{hpp,cpp}
│   │   ├── MaterialFactory.{hpp,cpp}
│   │   └── LightFactory.{hpp,cpp}
│   ├── parsers/
│   │   └── SceneParser.{hpp,cpp}   # libconfig++ scene + renderer parser
│   ├── utils/
│   │   ├── ConfigUtils.hpp         # Config parsing helpers
│   │   └── HelpDisplay.{hpp,cpp}   # Help text display
│   └── plugins/
│       ├── PluginMetadata.hpp      # Metadata struct (name, help, category)
│       ├── Shapes/                 # 13 shape implementations
│       │   ├── Sphere, Plane, Cylinder, Cone, Box, Rectangle,
│       │   ├── Triangle, Torus, Tanglecube, Hourglass,
│       │   └── LimitedCylinder, LimitedCone, LimitedHourglass
│       ├── Materials/              # 8 material implementations
│       │   ├── Lambertian, ColoredDiffuse, Phong, Reflective,
│       │   ├── Refractive, Chessboard, PerlinNoise, ImageTexture
│       └── Lights/                 # 2 light implementations
│           └── PointLight, DirectionalLight
├── plugins/                        # Built .so/.dylib files
│   ├── shapes/                     # 13 shape plugins
│   ├── materials/                  # 8 material plugins
│   └── lights/                     # 2 light plugins
├── scenes/                         # Example scene files (.txt)
├── textures/                       # PPM texture files
├── tests/                          # Unit tests (Criterion)
├── build/                          # CMake build directory
├── lib/deps/                       # FetchContent dependencies
├── help.txt                        # Auto-generated help from metadata
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
- Load scene via `SceneParser` (includes `RendererConfig`)
- Initialize `PluginManager` singleton
- Choose rendering method based on `RendererConfig` (SSAA antialiasing or none)
- Display `ProgressBar` during rendering
- Trace rays recursively through the scene (`trace()`)
- Handle direct lighting (shadows, diffuse, specular)
- Handle indirect lighting (reflections, refractions, max depth)
- Sample background (solid color or PPM image)
- Write final image to PPM (configurable output path)
- Optional logging via `Logger`

**Key Methods:**
```cpp
bool simulate();                                          // Main entry point: load, render, save
Image _render();                                          // Dispatch to AA or non-AA renderer
Image _renderNoAA();                                      // Standard 1-sample-per-pixel
Image _renderSSAA(int samples);                           // Supersampling antialiasing
Vec3 trace(const Ray& ray, int depth, double u, double v); // Recursive ray tracing
Vec3 _sampleBackground(double u, double v);               // Background color/image sampling
```

**Rendering Algorithm:**
1. For each pixel (x, y):
   - If antialiasing enabled: generate multiple jittered rays, average results
   - Otherwise: generate single ray from camera
   - Call `trace(ray, maxDepth, screenU, screenV)`
2. `trace()` logic:
   - Check if ray hits any object in World (AABB culling first)
   - If miss: sample background (color or image)
   - If hit:
     - Start with ambient lighting (configurable color/multiplier)
     - For each light source:
       - Cast shadow ray
       - If not in shadow: call `material->shade()` for diffuse/specular
     - If material scatters (reflective/refractive):
       - Recursively trace scattered ray (depth - 1)
       - Blend result with attenuation
3. Write pixel color to Image

### 2. Image (`src/core/Image.{hpp,cpp}`)

**Purpose:** Manages pixel buffer, PPM I/O, and texture sampling.

**Key Responsibilities:**
- Store RGB pixel data
- Clamp colors to [0, 255]
- Write PPM P3 format (ASCII)
- **NEW:** Read PPM files for textures
- **NEW:** Bilinear sampling at (u, v) coordinates for textures

**Key Methods:**
```cpp
void setPixel(int x, int y, const Vec3& color);
void write(const std::string& filename);
static Image readPPM(const std::string& filename);  // Load texture
Vec3 sample(double u, double v) const;              // Bilinear sampling
```

### 3. PluginManager (`src/core/PluginManager.{hpp,cpp}`)

**Purpose:** Singleton registry for all plugins. Scans, loads, and provides access to plugin create functions and metadata.

**Key Responsibilities:**
- Scan `plugins/{shapes,materials,lights}/` directories
- Load all `.so`/`.dylib` files via `PluginLoader`
- Resolve `create()` and `metadata()` symbols for each plugin
- Cache function pointers in maps
- Provide plugin discovery by category
- Generate help text from metadata

**Key Methods:**
```cpp
static PluginManager& instance();                        // Singleton accessor
void initialize();                                        // Scan and load all plugins
void* getCreateFunction(const std::string& typeName);    // Get plugin create function
std::vector<PluginMetadata> getPluginsByCategory(const std::string& category);
```

**Design Decision:** Centralized plugin management eliminates redundant loading across factories. Each plugin is loaded once, and factories query the manager for create functions.

### 4. AShape (`src/core/AShape.{hpp,cpp}`)

**Purpose:** Abstract base class providing automatic transformation and AABB optimization for bounded shapes.

**Key Features:**
- **Rotation:** Euler angles (rx, ry, rz) with ZYX convention
- **Translation:** Position offset (x, y, z)
- **AABB Culling:** Fast ray rejection before expensive intersection tests
- **Coordinate Transforms:** Automatic world ↔ local space conversions

**What AShape handles:**
- Transform rays from world to local space (shape at origin)
- Call derived class's `hitLocal()` for intersection
- Transform hit record back to world space
- AABB intersection test before `hitLocal()`
- Range validation (t_min/t_max)

**What derived classes implement:**
```cpp
bool hitLocal(const Ray& localRay, HitRecord& record) const;  // Intersection in local space
AABB computeLocalAABB() const;                                 // Bounding box at origin
```

**Example:** `Sphere` only implements sphere-at-origin intersection math. AShape handles rotation, translation, and AABB automatically.

### 5. Logger (`src/core/Logger.{hpp,cpp}`)

**Purpose:** Structured logging with timestamps for debugging and performance analysis.

**Key Methods:**
```cpp
void logScene(const std::string& scenePath, const Scene& scene);  // Scene details
void logTiming(double parseS, double renderS, double writeS, long long pixelCount);
void warn(const std::string& message);                            // Warning messages
```

**Output:** Writes to timestamped log file (e.g., `log_2026-04-27_15-30-45.txt`)

### 6. ProgressBar (`src/core/ProgressBar.{hpp,cpp}`)

**Purpose:** Real-time progress display during rendering.

**Features:**
- Visual progress bar (30 characters wide)
- Percentage complete
- Estimated time remaining
- Elapsed time on completion

### 7. SceneParser (`src/parsers/SceneParser.{hpp,cpp}`)

**Purpose:** Parse scene configuration files using libconfig++.

**Key Methods:**
```cpp
Scene parse(const std::string& filename);
void parseCamera(libconfig::Config& config, Scene& scene);
void parseRenderer(libconfig::Config& config, Scene& scene);  // NEW: Parse RendererConfig
void parseMaterials(libconfig::Config& config, std::unordered_map<std::string, std::shared_ptr<IMaterial>>& materialMap);
void parseShapes(libconfig::Config& config, const std::unordered_map<...>& materialMap, World& world);
void parseLights(libconfig::Config& config, std::vector<std::shared_ptr<ILight>>& lights);
```

**Design Decision:** Materials are parsed first and stored in a map. Shapes then reference materials by name. This allows material reuse across multiple shapes.

**New Parsing Sections:**
- **Renderer:** Antialiasing settings, lighting multipliers, background (color/image), output path
- **Rotation:** Bounded shapes (via AShape) now support optional rotation parameters

---

## Plugin System

### Overview

The raytracer uses a dynamic plugin system that loads `.so` (shared object) files at runtime. This allows shapes, materials, and lights to be added without recompiling the main executable.

### Components

**1. PluginLoader** (`src/core/PluginLoader.{hpp,cpp}`)
- RAII wrapper around libdl (`dlopen`, `dlsym`, `dlclose`)
- Manages plugin lifecycle (loading, symbol resolution, unloading)
- Stores loaded plugin handles in an `unordered_map<string, void*>`
- Cross-platform support (.so on Linux, .dylib on macOS)

**2. PluginManager** (`src/core/PluginManager.{hpp,cpp}`)
- Singleton pattern for centralized plugin registry
- Scans `plugins/` directory at startup via `initialize()`
- Loads all plugins once, caches function pointers
- Maps plugin names to `create()` and `metadata()` functions
- Provides category-based plugin discovery

**3. PluginMetadata** (`src/plugins/PluginMetadata.hpp`)
- Struct containing plugin information:
  ```cpp
  struct PluginMetadata {
      const char* pluginName;    // e.g., "sphere"
      const char* pluralForm;    // e.g., "spheres" (config section name)
      const char* helpText;      // e.g., "Sphere (position, radius, material, [rotation])"
      const char* category;      // "shape", "material", or "light"
  };
  ```

**4. Factories** (`src/factories/`)
- `ShapeFactory` - Creates shape plugins (static methods)
- `MaterialFactory` - Creates material plugins (static methods)
- `LightFactory` - Creates light plugins (static methods)
- Query `PluginManager` for create functions
- Parse libconfig settings
- Return `shared_ptr<IInterface>`

**5. Plugins** (`src/plugins/`)
- Located in `src/plugins/{Shapes,Materials,Lights}/` (source)
- Built to `plugins/{shapes,materials,lights}/` (binaries)
- Each plugin exports TWO `extern "C"` functions:
  - `create(...)` - Factory function
  - `metadata()` - Returns plugin metadata

### Plugin Contract

Each plugin must export TWO functions with C linkage:

```cpp
extern "C" IInterface* create(<plugin-specific-parameters>);
extern "C" PluginMetadata* metadata();  // or const PluginMetadata*
```

Where:
- `IInterface` is `IShape`, `IMaterial`, or `ILight`
- Parameters are plugin-specific (no void* or variadic args)
- Function names are always `"create"` and `"metadata"`
- `create()` returns a raw pointer; caller takes ownership
- `metadata()` returns pointer to static metadata struct

#### Plugin Examples

**Shape plugin (sphere.so):**
```cpp
// Sphere now inherits from AShape for automatic transformation
Sphere::Sphere(Vec3 rotation, Vec3 translation, double radius, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _radius(radius), _material(material) {}

extern "C" IShape* create(double rx, double ry, double rz,  // Rotation
                          double tx, double ty, double tz,  // Translation
                          double radius,
                          std::shared_ptr<IMaterial>* material) {
    return new Sphere(Vec3(rx, ry, rz), Vec3(tx, ty, tz), radius, *material);
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "sphere",
        .pluralForm = "spheres",
        .helpText = "Sphere (position (x, y, z), radius, material, [rotation (x, y, z)])",
        .category = "shape"
    };
    return &meta;
}
```

**Material plugin (reflective.so):**
```cpp
extern "C" IMaterial* create(double reflectivity, double r, double g, double b) {
    return new Reflective(reflectivity, Vec3(r, g, b));
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "reflective",
        .pluralForm = "reflective",
        .helpText = "Reflective (name, reflectivity, color (r, g, b))",
        .category = "material"
    };
    return &meta;
}
```

**Light plugin (pointlight.so):**
```cpp
extern "C" ILight* create(double x, double y, double z, double r, double g, double b, double intensity) {
    return new PointLight(Vec3(x, y, z), Vec3(r, g, b), intensity);
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "pointlight",
        .pluralForm = "point",
        .helpText = "PointLight (position (x, y, z), color (r, g, b), intensity)",
        .category = "light"
    };
    return &meta;
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

### Available Plugins (Current Implementation)

**Shapes (13):**
- **Bounded (AShape-based, support rotation):** Sphere, Box, Rectangle, Torus, Tanglecube, LimitedCylinder, LimitedCone, LimitedHourglass, Triangle
- **Infinite (IShape-based, custom orientation):** Plane, Cylinder, Cone, Hourglass

**Materials (8):**
- **Diffuse:** Lambertian, ColoredDiffuse
- **Specular:** Phong (shininess parameter)
- **Reflective:** Reflective (reflectivity parameter)
- **Refractive:** Refractive (opacity, refractiveIndex)
- **Procedural:** Chessboard (two colors, scale), PerlinNoise (color, scale)
- **Textured:** ImageTexture (PPM file path)

**Lights (2):**
- PointLight (position, color, intensity)
- DirectionalLight (direction, color, intensity)

### Adding New Plugins

1. **Create plugin source files:**
   ```
   src/plugins/Shapes/NewShape.cpp
   src/plugins/Shapes/NewShape.hpp
   ```

2. **Implement the interface:**
   ```cpp
   // For bounded shapes (with rotation/translation):
   class NewShape : public AShape {
   public:
       NewShape(Vec3 rotation, Vec3 translation, /*...params...*/, std::shared_ptr<IMaterial> material);
       bool hitLocal(const Ray& localRay, HitRecord& record) const override;
       AABB computeLocalAABB() const override;
   };

   // For infinite shapes (custom orientation):
   class NewShape : public IShape {
   public:
       NewShape(/*...params...*/, std::shared_ptr<IMaterial> material);
       bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;
   };

   extern "C" IShape* create(...params...) {
       return new NewShape(...);
   }

   extern "C" PluginMetadata* metadata() {
       static PluginMetadata meta = {
           .pluginName = "newshape",
           .pluralForm = "newshapes",
           .helpText = "NewShape (params...)",
           .category = "shape"
       };
       return &meta;
   }
   ```

3. **Add to CMakeLists.txt:**
   ```cmake
   add_raytracer_plugin(newshape shapes src/plugins/Shapes/NewShape.cpp)

   add_custom_target(shapes
       DEPENDS sphere box newshape  # Add here
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
       // Extract config, get create function from PluginManager, call plugin
   }
   ```

5. **Rebuild and test:**
   ```bash
   cmake --build build --target newshape
   ./raytracer scenes/test_newshape.txt
   ```

**Note:** With the metadata system, help text is automatically generated from plugins. No need to manually update help.txt.

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
- **NEW:** `double u` - Texture U coordinate (0-1)
- **NEW:** `double v` - Texture V coordinate (0-1)

**Key Method:**
```cpp
void set_face_normal(const Ray& r, const Vec3& outward_normal);
```
Determines if ray hits from inside or outside, flips normal accordingly.

**UV Coordinates:**
Used by procedural textures (Chessboard, PerlinNoise) and ImageTexture for surface mapping.

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
- **NEW:** `RendererConfig` (antialiasing, lighting, background, output)
- Material count (for logging)

### RendererConfig (`src/DataTypes/RendererConfig.hpp`)

**Purpose:** Centralized renderer settings.

**Fields:**
```cpp
struct RendererConfig {
    // Antialiasing
    bool aaEnabled = false;
    int aaSamples = 1;
    std::string aaMethod = "ssaa";  // Currently only SSAA supported

    // Lighting
    double ambientMultiplier = 0.4;
    double diffuseMultiplier = 0.6;
    Vec3 ambientColor = Vec3(25, 25, 38);

    // Background
    Vec3 backgroundColor = Vec3(135, 206, 235);  // Sky blue
    std::string backgroundImage = "";  // If set, overrides backgroundColor

    // Output
    std::string outputFile = "output.ppm";
};
```

**Design Decision:** Separating renderer settings from scene data allows easy experimentation with rendering quality without modifying the scene geometry.

---

## Build System

### CMake Structure

**Main CMakeLists.txt:**
- C++20 standard
- **Build Modes:**
  - **Debug:** `-g3 -O0` (full debug symbols, no optimization)
  - **Release:** `-O3 -march=native -ffast-math -funroll-loops -DNDEBUG` (aggressive optimizations)
- Compiler flags: `-Wall -Wextra -Werror`
- libconfig++ dependency (FetchContent fallback)
- Criterion for tests (FetchContent fallback)
- ImageMagick detection for texture conversion
- Plugin helper function: `add_raytracer_plugin()`
- Platform-specific plugin extensions (.so/.dylib)

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
# Renderer configuration (NEW - optional section)
renderer:
{
    antialiasing = {
        enabled = true;
        samples = 8;      # Number of samples per pixel
        method = "ssaa";  # Supersampling antialiasing
    };

    lighting = {
        ambientColor = { r = 25; g = 25; b = 38; };       # Dark blue-gray
        ambientMultiplier = 0.3;
        diffuseMultiplier = 0.7;
    };

    background = {
        color = { r = 135; g = 206; b = 235; };  # Sky blue
        image = "textures/space.ppm";            # Optional: overrides color
    };

    output = "output.ppm";  # Output file path
};

# Camera configuration
camera:
{
    resolution = { width = 1000; height = 1000; };
    position = { x = 0; y = -100; z = 20; };
    look_at = { x = 0; y = 0; z = 0; };
    up = { x = 0; y = 0; z = 1; };
    fieldOfView = 72.0;
};

# Materials (define once, reference by name)
materials:
{
    # Diffuse materials
    lambertian = (
        { name = "red"; color = { r = 255; g = 64; b = 64; }; }
    );

    # Reflective materials
    reflective = (
        { name = "mirror"; reflectivity = 0.9; color = { r = 255; g = 255; b = 255; }; }
    );

    # Phong materials (specular highlights)
    phong = (
        { name = "shiny"; color = { r = 60; g = 220; b = 60; }; shininess = 64.0; }
    );

    # Refractive materials (glass)
    refractive = (
        { name = "glass"; opacity = 0.2; refractiveIndex = 1.5; color = { r = 255; g = 255; b = 255; }; }
    );

    # Procedural textures
    chessboard = (
        { name = "board"; color1 = { r = 0; g = 0; b = 0; }; color2 = { r = 255; g = 255; b = 255; }; scale = 1.0; }
    );

    # Image textures
    imagetexture = (
        { name = "earth"; path = "textures/earth.ppm"; }
    );
};

# Shapes reference materials
shapes:
{
    # Spheres (support rotation)
    spheres = (
        {
            position = { x = 0; y = 0; z = 0; };
            radius = 25;
            material = "mirror";
            rotation = { x = 0; y = 0; z = 45; };  # Optional: Euler angles
        }
    );

    # Planes (infinite, custom orientation)
    planes = (
        {
            position = { x = 0; y = 0; z = -10; };
            normal = { x = 0; y = 0; z = 1; };
            material = "board";
        }
    );

    # Boxes (support rotation)
    boxes = (
        {
            position = { x = 10; y = 0; z = 0; };
            width = 5; height = 5; depth = 5;
            material = "red";
            rotation = { x = 0; y = 45; z = 0; };
        }
    );
};

# Lights
lights:
{
    point = (
        {
            position = { x = 400; y = 100; z = 500; };
            color = { r = 255; g = 255; b = 255; };
            intensity = 1.0;
        }
    );

    directional = (
        {
            direction = { x = -1; y = -1; z = -1; };
            color = { r = 200; g = 200; b = 200; };
            intensity = 0.5;
        }
    );
};
```

### Parser Flow

1. **Initialize PluginManager** → scan and load all plugins
2. **Parse renderer** → extract `RendererConfig` settings (optional, uses defaults if missing)
3. **Parse materials** → store in `std::unordered_map<string, shared_ptr<IMaterial>>`
4. **Parse shapes** → look up materials by name, create shapes with optional rotation
5. **Parse lights** → create light objects
6. **Parse camera** → set up view
7. **Load background image** → if `renderer.background.image` is specified

---

## Design Patterns

### 1. Singleton Pattern

**Used in:** PluginManager

**Purpose:** Ensure single instance of plugin registry, global access point.

**Implementation:**
```cpp
class PluginManager {
public:
    static PluginManager& instance() {
        static PluginManager _instance;
        return _instance;
    }
private:
    PluginManager() = default;  // Private constructor
};
```

**Benefits:**
- Plugins loaded once, shared across all factories
- Consistent plugin state throughout application lifetime

### 2. Factory Pattern

**Used in:** ShapeFactory, MaterialFactory, LightFactory

**Purpose:** Encapsulate object creation, allow runtime type selection.

**Implementation:**
- Static `create()` method takes type name and config
- Internal dispatch table maps type names to creator functions
- Each creator function queries PluginManager for create function pointer
- Parse libconfig settings, call plugin's `create()`, return `shared_ptr`

**Evolution:** Originally factories maintained their own PluginLoader instances. Now they query the centralized PluginManager singleton.

### 3. Strategy Pattern (via Interfaces)

**Used in:** IShape, IMaterial, ILight

**Purpose:** Define family of algorithms (hit detection, shading, illumination) and make them interchangeable.

**Example:**
```cpp
class IShape {
    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const = 0;
};

class Sphere : public AShape {  // Inherits from AShape (which implements IShape)
    bool hitLocal(...) const override { /* sphere intersection math in local space */ }
    AABB computeLocalAABB() const override { /* bounding box at origin */ }
};
```

### 4. Template Method Pattern

**Used in:** AShape

**Purpose:** Define skeleton of algorithm in base class, let subclasses override specific steps.

**Implementation:**
```cpp
class AShape : public IShape {
    // Template method (final - cannot be overridden)
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const final override {
        // 1. AABB culling
        if (!_worldAABB.hit(ray, t_min, t_max)) return false;
        // 2. Transform to local space
        Ray localRay = worldToLocal(ray);
        // 3. Call subclass implementation
        if (!hitLocal(localRay, record)) return false;
        // 4. Transform back to world space
        record = localToWorld(record, ray);
        // 5. Range validation
        return record.t >= t_min && record.t <= t_max;
    }

    // Primitive operations (overridden by subclasses)
    virtual bool hitLocal(const Ray& localRay, HitRecord& record) const = 0;
    virtual AABB computeLocalAABB() const = 0;
};
```

**Benefits:**
- Eliminates code duplication across all bounded shapes
- Automatic transformation and optimization
- Subclasses only implement shape-specific math

### 5. Plugin Pattern

**Used in:** Entire plugin system

**Purpose:** Allow functionality to be added at runtime without recompilation.

**Implementation:**
- Dynamic library loading with `dlopen`/`dlsym`
- `extern "C"` factory functions prevent name mangling
- Metadata system for plugin discovery and help generation

### 6. RAII (Resource Acquisition Is Initialization)

**Used in:** PluginLoader

**Purpose:** Automatic resource management (plugin handles).

**Implementation:**
- Constructor/`load()` calls `dlopen`
- Destructor calls `dlclose` on all handles
- No manual cleanup needed
- Exception-safe (handles closed even if exception thrown)

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
 ├─ Logger (unique_ptr)
 ├─ Background Image (unique_ptr)
 └─ Scene
     ├─ World (owns shapes via shared_ptr)
     ├─ Camera (value type)
     ├─ Lights (owns lights via shared_ptr)
     └─ RendererConfig (value type)
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

### Help System

**Location:** `src/utils/HelpDisplay.{hpp,cpp}` and `help.txt`

**Purpose:** Display available shapes, materials, and lights with their parameters.

**Invocation:**
```bash
./raytracer --help
# or
./raytracer
```

**Implementation:**
1. **Automatic Generation:** Help text is extracted from plugin metadata via PluginManager
2. **help.txt:** Pre-generated file containing all plugin documentation
3. **HelpDisplay:** Reads and formats help.txt for display

**help.txt Structure:**
```
Usage: raytracer <input_file> [--log]

Scene File Structure:
  Camera: resolution, position, look_at, up, fieldOfView
  Renderer (optional): antialiasing, lighting, background, output

Shapes:
  Box (position, width, height, depth, material, [rotation])
  Sphere (position, radius, material, [rotation])
  ... (all 13 shapes with parameters)

Materials:
  Lambertian (name, color)
  Phong (name, color, shininess)
  Reflective (name, reflectivity, color)
  ... (all 8 materials with parameters)

Lights:
  PointLight (position, color, intensity)
  DirectionalLight (direction, color, intensity)
```

**Design Decision:** Metadata-driven help ensures documentation stays in sync with code. When a plugin is added/removed, help text updates automatically.

**Logging:**
```bash
./raytracer scenes/example.txt --log
```
- Creates timestamped log file (e.g., `log_2026-04-27_15-30-45.txt`)
- Logs scene details, material count, renderer settings
- Logs timing: parse, render, write times
- Logs warnings (missing textures, etc.)

---

## Future Improvements

### Rendering Features

- [x] ~~Additional primitives: Plane, Cylinder, Cone, Torus~~ ✅ **DONE** (13 shapes implemented)
- [x] ~~Transformation matrix support (scale, rotation, shear)~~ ✅ **DONE** (rotation via AShape)
- [x] ~~Additional materials: Reflective, Refractive, Textured~~ ✅ **DONE** (8 materials)
- [x] ~~Phong reflection model (specular highlights)~~ ✅ **DONE** (Phong material)
- [x] ~~Antialiasing (supersampling, adaptive)~~ ✅ **DONE** (SSAA implemented)
- [ ] Ambient occlusion (soft shadows)
- [ ] Normal mapping
- [ ] MSAA (multisample antialiasing)
- [ ] Depth of field (camera focus)
- [ ] Motion blur
- [ ] Volumetric rendering (fog, smoke)
- [ ] Subsurface scattering

### Performance

- [x] ~~Progress bar during rendering~~ ✅ **DONE**
- [x] ~~Space partitioning (BVH, octree)~~ ✅ **PARTIAL** (AABB culling implemented)
- [ ] Full BVH tree construction
- [ ] Multithreading (parallel pixel rendering)
- [ ] SIMD vectorization for Vec3 operations
- [ ] GPU acceleration (CUDA/OpenCL)
- [ ] Adaptive sampling (focus rays on complex areas)
- [ ] Tile-based rendering

### Plugin System

- [x] ~~Plugin metadata: Version, author, description~~ ✅ **DONE** (PluginMetadata system)
- [x] ~~Plugin discovery: Auto-scan plugins directory~~ ✅ **DONE** (PluginManager)
- [ ] Error reporting: Better `dlerror()` messages
- [ ] Hot reloading: Unload/reload plugins at runtime
- [ ] Plugin validation: Check ABI compatibility
- [ ] Plugin versioning (major.minor.patch)
- [ ] Plugin dependencies (e.g., material requires specific shape)

### Materials & Textures

- [ ] Bump mapping / normal maps
- [ ] Emissive materials (light-emitting surfaces)
- [ ] Anisotropic materials (brushed metal)
- [ ] More procedural textures (marble, wood, noise)
- [ ] PNG/JPG texture support (currently PPM only)
- [ ] UV unwrapping for complex shapes

### Tooling

- [x] ~~Render time statistics~~ ✅ **DONE** (Logger with timing)
- [ ] GUI for real-time preview (SFML/Qt)
- [ ] Scene editor (visual)
- [ ] Automatic scene reload on file change
- [ ] Network rendering (distributed)
- [ ] Animation support (keyframes)
- [ ] Benchmark suite

---

## References

- **Project PDF**: `G-OOP-400_raytracer.pdf` - Complete specification
- **libconfig++**: https://hyperrealm.github.io/libconfig/
- **CMake**: https://cmake.org/cmake/help/latest/
- **Raytracing in One Weekend**: https://raytracing.github.io/ (conceptual reference)

---

## Quick Start

```bash
# Build everything (Release mode for best performance)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Or use Debug mode for development
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run with a scene
./raytracer scenes/gallery.txt

# With logging
./raytracer scenes/gallery.txt --log

# Output will be saved as specified in renderer.output (default: output.ppm)
# Convert to PNG (requires ImageMagick)
convert output.ppm output.png

# View help
./raytracer --help
```

### Performance Tips

1. **Always use Release mode** for final renders:
   - 5-10x faster than Debug mode
   - Enables `-O3 -march=native -ffast-math` optimizations

2. **Antialiasing trades quality for speed:**
   - `samples = 1`: Fastest, aliased edges
   - `samples = 4`: Good balance
   - `samples = 8-16`: High quality, slow

3. **AABB culling is automatic** for AShape-based shapes (most shapes)

4. **Limit recursion depth** in complex scenes with many reflective surfaces

### Example Workflow

```bash
# 1. Create/edit scene file
vim scenes/my_scene.txt

# 2. Quick preview (low quality, fast)
./raytracer scenes/my_scene.txt  # No AA, fast render

# 3. Final render (high quality)
# Edit scene: set renderer.antialiasing.samples = 8
./raytracer scenes/my_scene.txt --log

# 4. Convert to PNG
convert output.ppm final_render.png

# 5. Check log for performance stats
cat log_*.txt
```

---

## Project Statistics

- **Lines of Code:** ~10,000+ (including plugins)
- **Plugins Implemented:** 23 total (13 shapes, 8 materials, 2 lights)
- **Commits:** 363+ (as of April 2026)
- **Development Time:** ~4 months (January 2026 - April 2026)
- **C++ Standard:** C++20
- **Platform Support:** Linux, macOS

---

*This documentation was last updated on April 27, 2026, and reflects the current state of the raytracer project. Generated with assistance from Claude Code.*
