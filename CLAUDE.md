# Raytracer - Technical Reference

C++20 CPU raytracer, plugin-based, PNG/JPG/PPM output. 13 shapes, 8 materials, 2 lights.

## Architecture Flow

```
Main → Core → {SceneParser, Image, PluginManager}
         ↓
    trace() → AABB culling → hit detection → AO + lighting/materials → recursive scatter
         ↓
    ProgressBar, Logger, SSAA/Adaptive-SSAA antialiasing, RenderStats
```

**Core Components:**
- `Core`: Orchestrates rendering, trace() recursion, SSAA/Adaptive-SSAA, AO, shadow rays, background sampling. Tracks `RenderStats` (AO rays, shadow rays, bounces, samples)
- `RenderSampler`: Static helpers — `computeVariance()`, `randomHemisphere()` (used by adaptive SSAA + AO), `toneMapACES()` (luminance-preserving ACES filmic)
- `PluginManager`: Singleton, scans plugins/ dirs, caches create()/metadata() functions
- `AShape`: Template method pattern - handles world↔local transforms via `Matrix4x4`, AABB culling. `applyParentTransform()` for scene-in-scene composition. Subclasses implement hitLocal() + computeLocalAABB()
- `SceneParser`: libconfig++ parser, materials→map first, shapes reference by name. Recursive `_parseSubscenes()` with cycle detection and transform composition
- `Image`: Pixel buffer, PNG/JPG/PPM read+write via stb_image. `readFile()`/`writeFile()` dispatch by extension
- `Logger`: Timestamped logs, timing stats, render stats (AO/SSAA/shadow/bounce counters)
- `ProgressBar`: Real-time render feedback

## Directory Structure

```
src/
├── core/          Core, Image, RenderSampler, PluginLoader, PluginManager, AShape, Logger, ProgressBar
├── external/      stb_image.h, stb_image_write.h, stb.cpp (impl), Roots3And4.c
├── Interfaces/    IShape, IMaterial, ILight
├── DataTypes/     Vec3, Ray, HitRecord, Camera, World, Scene, RendererConfig, RenderStats
├── Math/          AABB, Matrix4x4, QuadraticSolver, CubicSolver, QuarticSolver
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
- **Materials (8):** Lambertian, ColoredDiffuse, Phong (shininess), Reflective (reflectivity), Refractive (opacity, refractiveIndex), Chessboard (colors, scale), PerlinNoise (color, scale), ImageTexture (PNG/JPG/PPM path)
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
    bool aaEnabled; int aaSamples; string aaMethod; // "ssaa" | "adaptive"
    double aaThreshold;
    bool aoEnabled; int aoSamples; double aoRadius;
    double ambientMultiplier, diffuseMultiplier;
    Vec3 ambientColor, backgroundColor;
    string backgroundImage, outputFile;
    bool multithreadingEnabled; int threadCount;
    bool   toneMappingEnabled  = true;  // toneMapping.enabled
    double toneMappingStrength = 0.8;   // toneMapping.strength [0-1]
};
```

**RenderStats:** Atomic counters — `aoRaysCast/Hit`, `ssaaSamples`, `adaptiveSamples/MaxSamples`, `shadowRaysCast/Hit`, `scatterBounces`. Logged via `Logger::logStats()` when `--log` active.

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

**Adaptive SSAA:** Recursive subdivision per pixel — samples 4 corners, checks variance against threshold. Subdivides high-variance regions up to `_maxSubdivDepth=2`. Tracks samples taken vs max possible in `RenderStats`.

**Ambient Occlusion:** Per-hit hemisphere sampling via `RenderSampler::randomHemisphere()`. `aoSamples` rays cast within `aoRadius`. Occlusion factor multiplies ambient term. Controlled by `RendererConfig.aoEnabled/aoSamples/aoRadius`.

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
renderer: { antialiasing{enabled,samples,method,threshold}, lighting{ambientColor,ambientMult,diffuseMult,ambientOcclusion{enabled,samples,radius}}, background{color,image}, output, toneMapping{enabled,strength} }
camera: { resolution{w,h}, position{x,y,z}, look_at{x,y,z}, up{x,y,z}, fieldOfView }
materials: { <type> = ( {name, ...params} ) }  # Define once, reference by name
shapes: { <plural> = ( {position/params, material, [rotation], [scale]} ) }
lights: { <plural> = ( {params} ) }
scenes: ( {path, position{x,y,z}, [rotation{x,y,z}], [scale{x,y,z}]} )  # Scene composition
```

**Parser Flow:**
1. PluginManager.initialize() → scan/load all plugins
2. Parse renderer → RendererConfig (optional, defaults)
3. Parse materials → unordered_map<string, shared_ptr<IMaterial>>
4. Parse shapes → lookup material by name, create with optional rotation/scale
5. Parse lights → create light objects
6. Parse camera → setup view
7. Parse subscenes → recursive, cycle detection via loading stack, transforms compose
8. Load background image if specified

**Scene Composition:** `scenes` section loads subscene files recursively. Each subscene has `position`/`rotation`/`scale` applied as parent transform. `renderer` and `camera` sections in subscenes are ignored. Materials merge into root map globally. AShape-based shapes get full transform composition; IShape-only shapes (Plane, Cylinder, Cone, Hourglass) get translation only with a warning.

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

**Logging:** `--log` creates timestamped log with scene details, timing (parse/render/write), render stats (AO/SSAA/shadow/bounce counters), warnings

## Performance

**Release mode:** 5-10x faster, use for final renders
**AA:** ssaa samples=1 (fast), 4 (balanced), 8-16 (slow HQ) | adaptive threshold=0.2 (loose), 0.05 (tight)
**AO:** samples=4 (fast, open scenes), 16 (balanced), 32+ (smooth, dense scenes). radius=2-4 (tight contact), 8+ (broad shadowing). Check `--log` stats: low hit% = reduce samples
**AABB culling:** Automatic for AShape shapes
**Limit recursion depth:** Avoid stack overflow with many reflective surfaces

## Debugging

**Rendering glitches (NaN artifacts, missing objects, incorrect normals):**
- Rebuild in Debug mode: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build`
- Run with debug build: `./raytracer scene.txt --log`
- Check for assertion failures, especially "Cannot normalize zero vector" in Vec3::normalize()
- Common causes: degenerate geometry, parallel camera vectors, accumulated floating-point error
- Debug builds enable assertions that catch divide-by-zero and invalid vector operations


*Stats: ~12k LOC, 23 plugins, 400+ commits, Jan-May 2026, C++20, Linux/macOS*
