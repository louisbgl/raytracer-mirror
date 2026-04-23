#pragma once

#include "../../Interfaces/IShape.hpp"

class Hourglass : public IShape {
public:
    Hourglass(Vec3 pos, Vec3 axis, double radius, std::shared_ptr<IMaterial> material);
    ~Hourglass() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    Vec3 _axis;
    double _radius;
    std::shared_ptr<IMaterial> _material;
};
