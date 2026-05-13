#pragma once

#include "core/AShape.hpp"

/**
 * @brief Abstract base class for ray-marched shapes (fractals, implicit surfaces, SDFs).
 *
 * What ARayMarchedShape handles for you:
 *   - Sphere tracing algorithm (ray marching loop)
 *   - Normal computation via gradient approximation
 *   - Configurable marching parameters (maxSteps, epsilon, maxDist)
 *   - Inherits transform/AABB/material from AShape
 *
 * What you implement:
 *   - distanceEstimator(): Signed distance function for your shape
 *   - computeLocalAABB(): Conservative bounding box for culling
 *
 * What you can assume:
 *   - distanceEstimator() receives points in local space (shape at origin)
 *   - Ray marching converges when distance < epsilon
 *   - Transforms handled by AShape automatically
 */
class ARayMarchedShape : public AShape {
public:
    ARayMarchedShape(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                     int maxSteps = 100, double epsilon = 1e-3, double maxDist = 100.0);
    virtual ~ARayMarchedShape() = default;

protected:
    int _maxSteps;
    double _epsilon;
    double _maxDist;

    /**
     * @brief Distance estimator function for the concrete shape.
     * @param point The point in local space to evaluate the distance from the surface.
     * @return The estimated distance to the surface. Positive outside, negative inside.
     */
    virtual double distanceEstimator(const Vec3& point) const = 0;

    /**
     * @brief Computes the normal at a point on the surface using numerical gradient of the distance field.
     * @param point The point on the surface in local space.
     * @return The normal vector at the given point.
     * @note This is a default implementation that can be overriden.
     */
    virtual Vec3 computeNormal(const Vec3& point) const;
    
    bool hitLocal(const Ray& ray, HitRecord& record) const override final;
    AABB computeLocalAABB() const override = 0;
};