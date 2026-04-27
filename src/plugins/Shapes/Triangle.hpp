#pragma once

#include "../../Interfaces/IShape.hpp"
#include "../../Math/AABB.hpp"

class Triangle : public IShape {
public:
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<IMaterial> material);
    ~Triangle() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;
    AABB boundingBox() const override { return _aabb; }

private:
    Vec3 _v0;
    Vec3 _edge1;
    Vec3 _edge2;
    Vec3 _normal;
    std::shared_ptr<IMaterial> _material;
    AABB _aabb;
    double _epsilon = 1e-8;
};