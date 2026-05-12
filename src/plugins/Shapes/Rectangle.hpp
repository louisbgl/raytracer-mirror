#pragma once

#include "../../core/AShape.hpp"

class Rectangle : public AShape {
public:
    Rectangle(Vec3 rotation, Vec3 translation, Vec3 scale, double width, double height, std::shared_ptr<IMaterial> material);
    ~Rectangle() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    double _width;
    double _height;
};
