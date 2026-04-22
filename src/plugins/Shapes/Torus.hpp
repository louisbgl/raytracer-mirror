#pragma once

#include "../../Interfaces/IShape.hpp"

class Torus : public IShape {
public:
    Torus(Vec3 position, double majorRadius, double minorRadius, std::shared_ptr<IMaterial> material);
    ~Torus() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _majorRadius;
    double _minorRadius;
    std::shared_ptr<IMaterial> _material;

    Vec3 computeNormal(const Vec3& point) const;
};