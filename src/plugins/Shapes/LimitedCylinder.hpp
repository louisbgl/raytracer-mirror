#pragma once

#include "../../core/AShape.hpp"

/**
 * @brief Represents a finite cylinder shape in the ray tracer.
 * @note pos represents the center of the cylinder's base.
 */
class LimitedCylinder : public AShape {
public:
    LimitedCylinder(Vec3 rotation, Vec3 translation, Vec3 scale, double radius, double height, std::shared_ptr<IMaterial> material);
    ~LimitedCylinder() override = default;

    bool hitLocal(const Ray& ray, HitRecord& record) const override;
    AABB computeLocalAABB() const override;

private:
    bool checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const;
    bool checkCapIntersection(const Ray& ray, double cap_y, const Vec3& normal, double t_min, double& closest_t, HitRecord& record) const;
    bool isWithinHeight(double y_coord) const;
    bool isWithinCapRadius(const Vec3& hit_point) const;
    Vec3 computeBodyNormal(const Vec3& hit_point) const;

    double _radius;
    double _height;
};