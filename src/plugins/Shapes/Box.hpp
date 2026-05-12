#pragma once

#include "../../core/AShape.hpp"

class Box : public AShape {
public:
    Box(Vec3 rotation, Vec3 translation, Vec3 scale, double width, double height, double depth, std::shared_ptr<IMaterial> material);
    ~Box() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    double _width;
    double _height;
    double _depth;
};
