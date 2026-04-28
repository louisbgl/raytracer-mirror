#pragma once

#include "IShape.hpp"
#include "../Math/AABB.hpp"

/**
 * @brief Opt-in interface for shapes that have a real axis-aligned bounding box.
 * Only finite shapes implement this. Infinite shapes (Plane, Cylinder, Cone...)
 * remain plain IShape and cannot be passed to a BVH.
 */
class IBoundable : public IShape {
public:
    virtual AABB boundingBox() const = 0;
};
