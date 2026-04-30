#pragma once

#include "../../core/AShape.hpp"

class Triangle : public AShape {
public:
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material);
    ~Triangle() override = default;

    bool hitLocal(const Ray& localRay, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    Vec3 _v0;
    Vec3 _edge1;
    Vec3 _edge2;
    Vec3 _normal;
    std::shared_ptr<IMaterial> _material;
    double _epsilon = 1e-8;
};
