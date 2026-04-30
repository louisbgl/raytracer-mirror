# Raytracer - Technical Reference

C++20 CPU raytracer, plugin-based, PPM output. 13 shapes, 8 materials, 2 lights.

## Architecture Flow

```
Main → Core → {SceneParser, Image, PluginManager}
         ↓
    trace() → AABB culling → hit detection → lighting/materials → recursive scatter
         ↓
    ProgressBar, Logger, SSAA antialiasing
```

**Core Components:**
- `Core`: Orchestrates rendering, trace() recursion, SSAA, shadow rays, background sampling
- `PluginManager`: Singleton, scans plugins/ dirs, caches create()/metadata() functions
- `AShape`: Template method pattern - handles world↔local transforms, AABB culling, rotation (Euler ZYX). Subclasses implement hitLocal() + computeLocalAABB()
- `SceneParser`: libconfig++ parser, materials→map first, shapes reference by name
- `Image`: Pixel buffer, PPM I/O, bilinear texture sampling
- `Logger`: Timestamped logs, timing stats
- `ProgressBar`: Real-time render feedback

## Directory Structure

```
src/
├── core/          Core, Image, PluginLoader, PluginManager, AShape, Logger, ProgressBar
├── Interfaces/    IShape, IMaterial, ILight
├── DataTypes/     Vec3, Ray, HitRecord, Camera, World, Scene, RendererConfig
├── Math/          AABB, QuadraticSolver, CubicSolver, QuarticSolver, Roots3And4.c
├── factories/     ShapeFactory, MaterialFactory, LightFactory
├── parsers/       SceneParser
├── utils/         ConfigUtils, HelpDisplay
└── plugins/       PluginMetadata, Shapes/, Materials/, Lights/

plugins/           Built .so/.dylib (shapes/, materials/, lights/)
```

## Plugin System

**Contract:** Each plugin exports:
```cpp
extern "C" IInterface* create(<typed-params>);
extern "C" PluginMetadata* metadata();
```

**PluginMetadata:**
```cpp
struct { const char *pluginName, *pluralForm, *helpText, *category; };
```

**Memory:** Plugins return raw `IInterface*`, factories wrap in `shared_ptr`. Materials passed as `shared_ptr<IMaterial>*` (avoid double-delete, allow refcount increment).

**Available Plugins:**
- **Shapes (13):** Sphere, Box, Rectangle, Torus, Tanglecube, LimitedCylinder, LimitedCone, LimitedHourglass, Triangle (AShape-based, rotation support) | Plane, Cylinder, Cone, Hourglass (IShape, custom orientation)
- **Materials (8):** Lambertian, ColoredDiffuse, Phong (shininess), Reflective (reflectivity), Refractive (opacity, refractiveIndex), Chessboard (colors, scale), PerlinNoise (color, scale), ImageTexture (PPM path)
- **Lights (2):** PointLight (pos, color, intensity), DirectionalLight (dir, color, intensity)

**Adding Plugin:**
1. Implement IInterface, extern "C" create()/metadata()
2. Add to CMakeLists: `add_raytracer_plugin(name category src/...)`
3. Add factory handler in {Shape,Material,Light}Factory.cpp
4. Rebuild: `cmake --build build --target <name>`

## Key Data Types

**Vec3:** constexpr 3D vector (positions, dirs, colors). Overloaded ops (+,-,*,/,dot,cross).

**Ray:** origin + direction, at(t) = origin + t*direction

**HitRecord:** {point, normal, t, front_face, material, u, v} - intersection data with UV coords

**Camera:** getRay(u,v) generates rays from view params (res, pos, look_at, up, FOV)

**World:** get_closest_hit() tests ray against all shapes

**Scene:** {World, Camera, Lights[], RendererConfig, materialCount}

**RendererConfig:**
```cpp
struct {
    bool aaEnabled; int aaSamples; string aaMethod; // "ssaa"
    double ambientMultiplier, diffuseMultiplier;
    Vec3 ambientColor, backgroundColor;
    string backgroundImage, outputFile;
};
```

**AABB:** Axis-aligned bounding box, slab method for fast ray culling before expensive intersection

## Rendering Algorithm

```cpp
// Per pixel:
if (AA) { jitter samples, average } else { single ray }
    ↓
trace(ray, depth, u, v):
    if (!world.hit()) return background.sample(u,v)
    color = ambient * ambientMultiplier
    for light:
        if (!shadow_ray.hit()) color += material.shade()
    if (material.scatter()) color += trace(scattered, depth-1) * attenuation
    return color
```

**SSAA:** Jittered grid sampling, averages N samples/pixel

## Build System

**CMake:**
- C++20, `-Wall -Wextra -Werror`
- Debug: `-g3 -O0` | Release: `-O3 -march=native -ffast-math -funroll-loops -DNDEBUG`
- libconfig++, Criterion (FetchContent fallback)
- `add_raytracer_plugin(name category src)` creates .so/.dylib in plugins/{category}/

**Targets:**
```bash
make raytracer        # Main exe
make plugins          # All plugins
make shapes/materials/lights  # Category
make <name>           # Single plugin
```

**Workflow:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./raytracer scenes/example.cfg [--log]
```

## Scene Config (libconfig++)

```
renderer: { antialiasing{enabled,samples,method}, lighting{ambientColor,ambientMult,diffuseMult}, background{color,image}, output }
camera: { resolution{w,h}, position{x,y,z}, look_at{x,y,z}, up{x,y,z}, fieldOfView }
materials: { <type> = ( {name, ...params} ) }  # Define once, reference by name
shapes: { <plural> = ( {position/params, material, [rotation]} ) }
lights: { <plural> = ( {params} ) }
```

**Parser Flow:**
1. PluginManager.initialize() → scan/load all plugins
2. Parse renderer → RendererConfig (optional, defaults)
3. Parse materials → unordered_map<string, shared_ptr<IMaterial>>
4. Parse shapes → lookup material by name, create with optional rotation
5. Parse lights → create light objects
6. Parse camera → setup view
7. Load background image if specified

## Design Patterns

**Singleton:** PluginManager - single plugin registry, global access
**Factory:** Static create() methods, dispatch tables, query PluginManager
**Strategy:** IShape/IMaterial/ILight interfaces, interchangeable algorithms
**Template Method:** AShape.hit() final, subclasses override hitLocal()/computeLocalAABB()
**Plugin:** dlopen/dlsym dynamic loading, extern "C" prevents mangling, metadata for discovery
**RAII:** PluginLoader auto dlclose on destruction

## Memory Management

**Ownership:** Core → {Logger, Background} → Scene → {World (shapes), Camera, Lights, RendererConfig}
**Shared:** Materials via shared_ptr, multiple shapes reference same instance
**Plugin Boundary:** Plugins return raw*, factories wrap in shared_ptr immediately

## Critical Design Decisions

**shared_ptr<IMaterial>* across plugin boundary:** Materials shared in map, passing pointer allows refcount increment without double-delete
**extern "C":** Prevents name mangling, dlsym() can find "create"
**No IPlugin interface:** Config sections already separate types, parser knows factory to call
**No void* params:** Type-safe factory signatures, compile-time error checking
**Materials parsed first:** Build map, shapes reference by name (reuse)
**AShape template method:** Eliminates duplication, auto transform/AABB for all bounded shapes

## CLI

**Usage:** `./raytracer <scene.cfg> [--log]` | `./raytracer --help`

**Help:** Metadata-driven, auto-generated from plugin metadata, displays all params

**Logging:** `--log` creates timestamped log with scene details, timing (parse/render/write), warnings

## Performance

**Release mode:** 5-10x faster, use for final renders
**AA:** samples=1 (fast), 4 (balanced), 8-16 (slow HQ)
**AABB culling:** Automatic for AShape shapes
**Limit recursion depth:** Avoid stack overflow with many reflective surfaces

## Debugging

**Rendering glitches (NaN artifacts, missing objects, incorrect normals):**
- Rebuild in Debug mode: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build`
- Run with debug build: `./raytracer scene.txt --log`
- Check for assertion failures, especially "Cannot normalize zero vector" in Vec3::normalize()
- Common causes: degenerate geometry, parallel camera vectors, accumulated floating-point error
- Debug builds enable assertions that catch divide-by-zero and invalid vector operations

## Adding OBJ File Support (Implementation Guide)

**New Plugin: Mesh (shape)**

1. **Create Mesh shape:**
   - `src/plugins/Shapes/Mesh.{hpp,cpp}`
   - Inherits AShape (rotation/translation/AABB support)
   - Stores vector<Triangle> (reuse existing Triangle intersection)
   - hitLocal(): test ray against all triangles, return closest
   - computeLocalAABB(): bbox encompassing all vertices

2. **OBJ Parser:**
   - `src/utils/ObjParser.{hpp,cpp}`
   - Parse v/vt/vn/f lines
   - Return vertices[], uvs[], normals[], faces[] (indices)
   - Handle triangulation (quads → 2 triangles)

3. **Mesh implementation:**
```cpp
class Mesh : public AShape {
    vector<Triangle> _triangles;
    AABB _localAABB;
public:
    Mesh(Vec3 rot, Vec3 trans, string objPath, shared_ptr<IMaterial> mat);
    bool hitLocal(const Ray& ray, HitRecord& rec) const override {
        // Test all triangles, keep closest hit
    }
    AABB computeLocalAABB() const override { return _localAABB; }
};

extern "C" IShape* create(double rx, ry, rz, tx, ty, tz,
                          const char* path, shared_ptr<IMaterial>* mat) {
    return new Mesh(Vec3(rx,ry,rz), Vec3(tx,ty,tz), path, *mat);
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "mesh",
        .pluralForm = "meshes",
        .helpText = "Mesh (position, objPath, material, [rotation])",
        .category = "shape"
    };
    return &meta;
}
```

4. **Config syntax:**
```
meshes = (
    { position = {x,y,z}; path = "models/bunny.obj"; material = "red"; rotation = {0,45,0}; }
);
```

5. **Optimization:**
   - BVH tree for meshes with many triangles (>1000)
   - Shared vertices (indexed geometry)
   - Normal interpolation for smooth shading

6. **CMakeLists.txt:**
```cmake
add_raytracer_plugin(mesh shapes src/plugins/Shapes/Mesh.cpp src/utils/ObjParser.cpp)
add_custom_target(shapes DEPENDS sphere box mesh ...)
```

7. **Factory handler:** Add _createMesh() to ShapeFactory.cpp, parse path param

---

*Stats: ~10k LOC, 23 plugins, 363+ commits, Jan-Apr 2026, C++20, Linux/macOS*
