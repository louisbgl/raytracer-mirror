#pragma once

#include "../../core/AShape.hpp"

class Sphere : public AShape {
public:
    Sphere(Vec3 rotation, Vec3 translation, Vec3 scale, double radius, std::shared_ptr<IMaterial> material);
    ~Sphere() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    double _radius;
};