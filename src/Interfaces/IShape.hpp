#pragma once

#include "../DataTypes/Ray.hpp"
#include "../DataTypes/HitRecord.hpp"
#include "../Math/AABB.hpp"

/**
 * @brief Interface for shapes that can be hit by rays.
 * @note Each shape must implement its own way of knowing its material.
 * This can be done thru the contructor, for example.
 */
class IShape {
public:
    virtual ~IShape() = default;

    /**
     * @brief Checks if a ray hits the shape within the specified t-range.
     * @param ray The ray to check.
     * @param t_min The minimum t-value.
     * @param t_max The maximum t-value.
     * @param record [OUT] The hit record to fill if a hit is found.
     * @return True if a hit is found, false otherwise.
     */
    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const = 0;
    /**
     * @brief Returns the axis-aligned bounding box of this shape.
     * @note Infinite shapes (plane, cylinder...) don't have a meaningful AABB,
     *       so the default returns an empty box. Only shapes used inside a BVH
     *       (e.g. Triangle) need to override this.
     */
    virtual AABB boundingBox() const { return AABB{}; }
};