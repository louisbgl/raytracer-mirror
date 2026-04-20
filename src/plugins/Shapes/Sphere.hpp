#pragma once

#include "../../Interfaces/IShape.hpp"

class Sphere : public IShape {
public:
    Sphere(Vec3 pos, double radius, std::shared_ptr<IMaterial> material);
    ~Sphere() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _position;
    double _radius;
    std::shared_ptr<IMaterial> _material;
};