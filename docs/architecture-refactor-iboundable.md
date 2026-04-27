# Architecture Refactor: IBoundable Interface

## Problem

The current architecture has three interconnected problems:

1. **`IShape::boundingBox()` is non-pure virtual with a silent default.** Any shape — including infinite ones like Plane or Cylinder — can be silently passed to a BVH. The compiler raises no warning. The BVH accepts the shape with an empty `AABB{}`, and the shape is never hit. This is a silent correctness bug.

2. **No type-level distinction between bounded and infinite shapes.** `IBoundable` and `IInfinite` are the same type: `IShape`. The type system cannot prevent misuse.

3. **`Mesh` and `Triangle` have no transform support.** Both inherit `IShape` directly. They cannot be rotated, translated, or scaled in scene configs.

---

## Decided Architecture

### New Hierarchy

```
IShape
  └── hit() — pure virtual ONLY (no boundingBox)

IBoundable : IShape
  └── boundingBox() — pure virtual
      Opt-in contract: "this shape has a real AABB."
      Compiler prevents infinite shapes from entering a BVH.

AShape : IBoundable                          ← finite, transformable scene shapes
  ├── hit() — FINAL (transform pipeline)
  ├── boundingBox() — implemented (transformAABB(computeLocalAABB()))
  ├── hitLocal() — pure virtual
  └── computeLocalAABB() — pure virtual
      │
      ├── Sphere, Box, Rectangle, LimitedCylinder, LimitedCone,
      │   LimitedHourglass, Torus, Tanglecube          ← unchanged
      │
      ├── Triangle (AShape) ← promoted                 ← scene-config triangle, has transforms
      │   hitLocal(): Möller–Trumbore in local space
      │   computeLocalAABB(): from v0/v1/v2
      │
      └── Mesh (AShape) ← promoted                     ← rotatable/scalable/translatable
          hitLocal(): delegates to internal BVHNode
          computeLocalAABB(): returns BVHNode::boundingBox()

BVHNode : IBoundable                                    ← pure accelerator, not a scene shape
  └── constructor takes vector<shared_ptr<IBoundable>> ← type-safe

MeshTriangle : IBoundable                               ← internal to Mesh/ObjParser
  └── NOT AShape — no per-triangle transform overhead  ← performance-critical (900k+ instances)
      stores Mesh-local-space vertices
      keeps UV + vertex normal support (for OBJ import)
```

### Infinite Shapes — Untouched

`Plane, Cylinder, Cone, Hourglass` remain direct `IShape` descendants.  
They do **not** implement `IBoundable`. No code changes needed.  
The compiler now makes it impossible to pass them to a `BVHNode`. ✓

---

## Key Decisions and Reasoning

### Why IBoundable and not pure virtual in IShape?

Making `boundingBox()` pure virtual in `IShape` would force `Plane`, `Cylinder`, `Cone`, and `Hourglass` to implement it — either with a stub or an assert. That forces a contract onto shapes that genuinely have no bounding box. `IBoundable` is the honest design: opt-in, not forced.

### Why Triangle becomes AShape

Scene-config triangles should behave like `Rectangle` — supporting position and rotation in config files. `AShape` gives this for free. The three vertices define the local-space shape; `AShape`'s transform pipeline moves and rotates it in world space.

### Why MeshTriangle is IBoundable (not AShape)

A mesh with 900k triangles means 900k `hit()` calls per ray traversal. `AShape::hit()` always runs `worldToLocal` + `localToWorld` matrix operations. For mesh-internal triangles, the transform happens **once** at `Mesh` level (Mesh is AShape). Applying the AShape pipeline to every `MeshTriangle` would be a serious performance regression. `MeshTriangle` stays lean: pure geometry, no transform overhead.

### Why Mesh becomes AShape

The `AShape` transform runs once per ray before BVH traversal. The inverse-transformed ray is passed into the BVH. `MeshTriangle` vertices are stored in Mesh-local space (as `ObjParser` produces them). The chain is:

```
World ray → Mesh::hit() (AShape pipeline) → localRay → BVHNode → MeshTriangle::hit()
```

This means meshes become fully transformable in scene configs: `position`, `rotation`.

---

## Impact on Scene Config

### Triangle — Before and After

**Before (no transforms):**
```
triangles = (
    { v0 = {x=0;y=0;z=0.}; v1 = {x=1;y=0;z=0.}; v2 = {x=0;y=1;z=0.}; material = "red"; }
);
```

**After (AShape — works like Rectangle):**
```
triangles = (
    { v0 = {x=0;y=0;z=0.}; v1 = {x=1;y=0;z=0.}; v2 = {x=0;y=1;z=0.};
      position = {x=5;y=0;z=0.}; rotation = {x=0;y=45;z=0.};
      material = "red"; }
);
```

### Mesh — Before and After

**Before (no transforms):**
```
meshes = (
    { path = "textures/obj/bugatti/bugatti.obj"; material = "carbody"; }
);
```

**After (AShape — rotation and position work):**
```
meshes = (
    { path = "textures/obj/bugatti/bugatti.obj"; material = "carbody";
      position = {x=0;y=0;z=0.}; rotation = {x=0;y=45;z=0.}; }
);
```

---

## Future World BVH

`World` currently stores `vector<shared_ptr<IShape>>` and iterates linearly.  
When the world-level BVH is implemented:

1. Separate shapes into `vector<shared_ptr<IBoundable>>` (finite) and a `vector<shared_ptr<IShape>>` (infinite only)
2. Build a single `BVHNode` from the bounded shapes
3. Per ray: test the BVH first, then test infinite shapes linearly

This is safe and correct because `IBoundable` guarantees a real AABB exists. The architecture supports this without further interface changes.

---

## Files Affected

| File | Change |
|------|--------|
| `src/Interfaces/IBoundable.hpp` | **Create** — new interface |
| `src/Interfaces/IShape.hpp` | Remove `boundingBox()` default |
| `src/core/AShape.hpp` | Inherit `IBoundable`, add `boundingBox()` |
| `src/core/AShape.cpp` | Implement `boundingBox()` |
| `src/Math/BVHNode.hpp` | Inherit `IBoundable`, constructor takes `vector<shared_ptr<IBoundable>>` |
| `src/plugins/Shapes/Triangle.hpp/.cpp` | Rewrite as AShape (`hitLocal` + `computeLocalAABB`) |
| `src/plugins/Shapes/MeshTriangle.hpp/.cpp` | **Create** — current Triangle logic, IBoundable, keeps UV/vn |
| `src/plugins/Shapes/Mesh.hpp/.cpp` | Rewrite as AShape |
| `src/parsers/ObjParser.hpp/.cpp` | Returns `vector<shared_ptr<IBoundable>>` of `MeshTriangle` |
| `src/factories/ShapeFactory.cpp` | Update Triangle creation for AShape signature |
| `CMakeLists.txt` | Add `MeshTriangle` target |
| Infinite shapes | **No changes** |
| All other AShape shapes | **No changes** |
