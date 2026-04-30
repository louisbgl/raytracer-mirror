#pragma once

#include "../../core/AShape.hpp"

class LimitedCone : public AShape {
public:
    LimitedCone(Vec3 rotation, Vec3 translation, Vec3 scale, double radius, double height, std::shared_ptr<IMaterial> material);
    ~LimitedCone() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    bool checkBaseIntersection(const Ray& ray, double& closest_t, HitRecord& record) const;

    double _radius;
    double _height;
    std::shared_ptr<IMaterial> _material;
};