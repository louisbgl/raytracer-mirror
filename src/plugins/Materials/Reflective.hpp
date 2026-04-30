#pragma once

#include "../../Interfaces/IMaterial.hpp"

/**
 * @brief Reflective material implementing perfect mirror reflection.
 *
 * Creates mirror-like surfaces that reflect incoming rays according to the law of reflection.
 * The reflected ray direction is computed as: r = d - 2(d·n)n
 *
 * @param reflectivity How reflective the surface is (0 = no reflection, 1 = perfect mirror)
 * @param color Tint applied to reflected light (RGB 0-255)
 */
class Reflective : public IMaterial {
public:
    Reflective(double reflectivity, Vec3 color);

    Vec3 shade([[maybe_unused]] const HitRecord& record, [[maybe_unused]] const Vec3& lightDir,
               [[maybe_unused]] const Vec3& lightColor,  [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const override;
    Vec3 shadowTransmittance() const override;

private:
    double _reflectivity;
    Vec3 _color;

    static double _clampReflectivity(double reflectivity);
};