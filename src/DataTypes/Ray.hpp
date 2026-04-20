#pragma once

#include "Vec3.hpp"

/**
 * @brief Represents a ray in 3D space, defined by an origin and a direction.
 * @note The direction is normalized upon construction to ensure consistent behavior.
 */
class Ray {
public:
    Ray() = default;
    Ray(const Vec3& origin, const Vec3& direction) : _origin(origin), _direction(normalize(direction)) {}

    const Vec3& origin() const { return _origin; }
    const Vec3& direction() const { return _direction; }

    /**
     * @brief Computes the point along the ray at parameter t.
     * @param t The parameter along the ray.
     * @return The point at parameter t.
     */
    Vec3 at(double t) const {
        return _origin + t * _direction;
    }

private:
    Vec3 _origin;
    Vec3 _direction;
};