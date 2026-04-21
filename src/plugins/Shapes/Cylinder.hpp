#pragma once

#include "../../Interfaces/IShape.hpp"

class Cylinder : public IShape {
public:
    Cylinder(Vec3 pos, double radius, double height, std::shared_ptr<IMaterial> material);
    ~Cylinder() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    bool checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const;
    bool checkCapIntersection(const Ray& ray, double cap_y, const Vec3& normal, double t_min, double& closest_t, HitRecord& record) const;
    bool isWithinHeight(double y_coord) const;
    bool isWithinCapRadius(const Vec3& hit_point) const;
    Vec3 computeBodyNormal(const Vec3& hit_point) const;

    Vec3 _position;
    double _height;
    double _radius;
    std::shared_ptr<IMaterial> _material;
};