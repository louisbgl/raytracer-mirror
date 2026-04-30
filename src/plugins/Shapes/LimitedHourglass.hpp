#pragma once

#include "../../core/AShape.hpp"

class LimitedHourglass : public AShape {
public:
    LimitedHourglass(Vec3 rotation, Vec3 translation, Vec3 scale, double radius, double height, std::shared_ptr<IMaterial> material);
    ~LimitedHourglass() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    bool checkCapIntersection(const Ray& ray, double cap_y, double& closest_t, HitRecord& record) const;

    double _radius;
    double _height;
    std::shared_ptr<IMaterial> _material;
};