#pragma once

#include "../../Interfaces/IMaterial.hpp"

/**
 * @brief Refractive material implementing light bending and transparency.
 *
 * Combines transparency with physically-based refraction using Snell's law.
 * Light bends when entering/exiting the material based on the refractive index.
 * Handles total internal reflection at grazing angles.
 *
 * @param opacity Controls material opacity (0 = fully transparent, 1 = opaque)
 * @param refractiveIndex Index of refraction (1.0 = air, 1.33 = water, 1.5 = glass, 2.4 = diamond)
 * @param color Tint applied to light passing through (RGB 0-255)
 */
class Refractive : public IMaterial {
public:
    Refractive(double opacity, double refractiveIndex, Vec3 color);

    Vec3 shade([[maybe_unused]] const HitRecord& record, [[maybe_unused]] const Vec3& lightDir,
               [[maybe_unused]] const Vec3& lightColor,  [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const override;
    Vec3 shadowTransmittance() const override;

private:
    double _opacity;
    double _refractiveIndex;
    Vec3 _color;

    Vec3 reflect(const Vec3& v, const Vec3& n) const;
    Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat) const;
};