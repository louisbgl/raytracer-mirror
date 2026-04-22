#pragma once

#include "../../Interfaces/IShape.hpp"

/**
 * @brief Represents a finite cylinder shape in the ray tracer.
 * @note pos represents the center of the cylinder's base, and the cylinder extends upwards along the y-axis.
 */
class LimitedCylinder : public IShape {
public:
    LimitedCylinder(Vec3 pos, double radius, double height, std::shared_ptr<IMaterial> material);
    ~LimitedCylinder() override = default;

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override;

private:
    bool checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const;
    bool checkCapIntersection(const Ray& ray, double cap_y, const Vec3& normal, double t_min, double& closest_t, HitRecord& record) const;
    bool isWithinHeight(double y_coord) const;
    bool isWithinCapRadius(const Vec3& hit_point) const;
    Vec3 computeBodyNormal(const Vec3& hit_point) const;

    Vec3 _position;
    double _radius;
    double _height;
    std::shared_ptr<IMaterial> _material;
};