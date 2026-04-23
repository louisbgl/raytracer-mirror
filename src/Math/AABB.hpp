#pragma once

#include "../DataTypes/Vec3.hpp"
#include "../DataTypes/Ray.hpp"
#include <algorithm>

/**
 * @brief Axis-Aligned Bounding Box for fast ray intersection tests
 */
class AABB {
public:
    AABB() = default;
    AABB(const Vec3& a, const Vec3& b)
        : _min(Vec3(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()))),
          _max(Vec3(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()))) {}

    /**
     * @brief Fast ray-box intersection test (slab method)
     * @param ray The ray to test
     * @param t_min Minimum t value
     * @param t_max Maximum t value  
     * @return true if ray intersects box in range [t_min, t_max]
     */
    bool hit(const Ray& ray, double t_min, double t_max) const {
        for (int axis = 0; axis < 3; axis++) {
            // Handle near-zero direction (axis-aligned ray)
            if (std::abs(ray.direction()[axis]) < _epsilon) {
                if (ray.origin()[axis] < _min[axis] || ray.origin()[axis] > _max[axis])
                    return false;
                continue;
            }

            double invD = 1.0 / ray.direction()[axis];
            double t0 = (_min[axis] - ray.origin()[axis]) * invD;
            double t1 = (_max[axis] - ray.origin()[axis]) * invD;

            if (invD < 0.0) std::swap(t0, t1);

            t_min = std::max(t0, t_min);
            t_max = std::min(t1, t_max);

            if (t_max <= t_min) return false;
        }
        return true;
    }

    /**
     * @brief Combine two bounding boxes
     */
    static AABB surrounding_box(const AABB& box0, const AABB& box1) {
        Vec3 small(std::min(box0.min().x(), box1.min().x()),
                   std::min(box0.min().y(), box1.min().y()),
                   std::min(box0.min().z(), box1.min().z()));
        Vec3 big(std::max(box0.max().x(), box1.max().x()),
                 std::max(box0.max().y(), box1.max().y()),
                 std::max(box0.max().z(), box1.max().z()));
        return AABB(small, big);
    }

    constexpr Vec3 min() const { return _min; }
    constexpr Vec3 max() const { return _max; }

private:
    Vec3 _min;
    Vec3 _max;
    static constexpr double _epsilon = 1e-8;
};