#pragma once

#include "../Interfaces/IBoundable.hpp"
#include "../Math/AABB.hpp"
#include "../Math/Matrix4x4.hpp"

/**
 * @brief Abstract base class providing automatic transformation and AABB optimization for all shapes.
 *
 * What AShape handles for you:
 *   - Rotation and translation transformations
 *   - AABB-based early ray rejection
 *   - Range validation (t_min/t_max)
 *   - Ray space conversions (world ↔ local)
 *
 * What you implement:
 *   - hitLocal(): Ray intersection assuming shape is at origin (0,0,0) and axis-aligned
 *   - computeLocalAABB(): Bounding box for shape at origin
 * 
 * What you can assume:
 *  - The concrete shape's positon will be 0, 0, 0
 *  - The concrete shape will be axis-aligned
 */
class AShape : public IBoundable {
public:
    /**
     * @brief Constructor for specifying rotation, translation, and scale separately.
     * @param rotation Euler angles in degrees (rx, ry, rz) - ZYX convention
     * @param translation Position in world space (x, y, z)
     * @param scale Scale factors for each axis (x, y, z)
     */
    AShape(Vec3 rotation, Vec3 translation, Vec3 scale);

    /**
     * @brief Constructor for directly providing a transformation matrix.
     * @param transform The world transformation matrix to apply to the shape.
     */
    AShape(const Matrix4x4& transform);

    /**
     * @brief [FINAL] Full intersection pipeline: AABB check → transform to local → hitLocal() → transform to world → range check
     */
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const final override;
    AABB boundingBox() const override;

    /**
     * @brief Intersection test in local space (shape at origin, axis-aligned). No need to check t_min/t_max.
     */
    virtual bool hitLocal(const Ray& localRay, HitRecord& record) const = 0;

    /**
     * @brief Return axis-aligned bounding box for shape at origin (local space).
     */
    virtual AABB computeLocalAABB() const = 0;

protected:
    void updateWorldAABB();

private:
    Matrix4x4 _transform;           // World transformation matrix (rotation + translation)
    Matrix4x4 _inverseTransform;    // Inverse of the world transformation (for ray conversion)
    Matrix4x4 _normalTransform;     // Inverse transpose for transforming normals (if needed)
    mutable AABB _worldAABB;        // Transformed bounding box in world space
    mutable bool _aabbNeedsUpdate;

    Ray worldToLocal(const Ray& ray) const;
    HitRecord localToWorld(const HitRecord& local, const Ray& worldRay) const;
    AABB transformAABB(const AABB& localAABB) const;
};