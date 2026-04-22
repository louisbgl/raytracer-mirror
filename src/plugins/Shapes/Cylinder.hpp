#pragma once

#include "../../Interfaces/IShape.hpp"

/**
 * @brief Represents an infinite cylinder shape in the ray tracer.
 * @note pos represents the center point on the cylinder's axis, and the cylinder extends infinitely along the y-axis.
 */
class Cylinder : public IShape {
public:
    Cylinder(Vec3 pos, double radius, std::shared_ptr<IMaterial> material);
    ~Cylinder() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    bool checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const;
    Vec3 computeBodyNormal(const Vec3& hit_point) const;

    Vec3 _position;
    double _radius;
    std::shared_ptr<IMaterial> _material;
};