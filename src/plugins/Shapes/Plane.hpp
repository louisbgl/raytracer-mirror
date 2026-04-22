#pragma once

#include "../../Interfaces/IShape.hpp"

class Plane : public IShape {
public:
    Plane(Vec3 pos, Vec3 normal, std::shared_ptr<IMaterial> material);
    ~Plane() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    Vec3 _pos;
    Vec3 _normal;
    std::shared_ptr<IMaterial> _material;
};