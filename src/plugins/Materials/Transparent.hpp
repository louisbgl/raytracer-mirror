#pragma once

#include "../../Interfaces/IMaterial.hpp"

class Transparent : public IMaterial {
public:
    Transparent(double opacity, double refractiveIndex, Vec3 color);

    Vec3 shade([[maybe_unused]] const HitRecord& record, [[maybe_unused]] const Vec3& lightDir,
               [[maybe_unused]] const Vec3& lightColor,  [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const override;

private:
    [[maybe_unused]] double _opacity;
    double _refractiveIndex;
    Vec3 _color;

    Vec3 reflect(const Vec3& v, const Vec3& n) const;
    Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat) const;
};