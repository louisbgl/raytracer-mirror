#pragma once

#include "../../Interfaces/IMaterial.hpp"

class Transparent : public IMaterial {
public:
    Transparent(double opacity, double refractiveIndex, Vec3 color);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor) const override;
    bool scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const override;

private:
    double _opacity;
    double _refractiveIndex;
    Vec3 _color;
};