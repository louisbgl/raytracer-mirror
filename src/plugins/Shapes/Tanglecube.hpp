#pragma once

#include "../../core/AShape.hpp"

class Tanglecube : public AShape {
public:
    Tanglecube(Vec3 rotation, Vec3 translation, double scale, std::shared_ptr<IMaterial> material);
    ~Tanglecube() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    double _scale;
    std::shared_ptr<IMaterial> _material;

    double evaluate(double x, double y, double z) const;
    Vec3 gradient(double x, double y, double z) const;
};
