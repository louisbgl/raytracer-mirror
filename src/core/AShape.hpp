#pragma once

#include "../Interfaces/IShape.hpp"
#include "../Math/AABB.hpp"

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
 */
class AShape : public IShape {
public:
    /**
     * @param rotation Euler angles in degrees (rx, ry, rz) - ZYX convention
     * @param translation Position in world space (x, y, z)
     */
    AShape(Vec3 rotation, Vec3 translation);

    /**
     * @brief [FINAL] Full intersection pipeline: AABB check → transform to local → hitLocal() → transform to world → range check
     */
    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const final override;

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
    Vec3 _rotation;      // Euler angles for rotation
    Vec3 _translation;   // Translation offset
    AABB _worldAABB;     // Transformed bounding box in world space

    Ray worldToLocal(const Ray& ray) const;
    HitRecord localToWorld(const HitRecord& local) const;
    AABB transformAABB(const AABB& localAABB) const;

    // Helper methods for 3D rotation math
    Vec3 applyRotation(const Vec3& v, double rx, double ry, double rz) const;
    Vec3 applyInverseRotation(const Vec3& v, double rx, double ry, double rz) const;
};