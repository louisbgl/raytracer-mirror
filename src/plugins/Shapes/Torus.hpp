#pragma once

#include "../../core/AShape.hpp"

class Torus : public AShape {
public:
    Torus(Vec3 rotation, Vec3 translation, double majorRadius, double minorRadius, std::shared_ptr<IMaterial> material);
    ~Torus() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    double _majorRadius;
    double _minorRadius;
    std::shared_ptr<IMaterial> _material;

    Vec3 computeNormal(const Vec3& point) const;
};