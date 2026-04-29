#include "Refractive.hpp"
#include "../PluginMetadata.hpp"

#include <cmath>

#include "../../DataTypes/HitRecord.hpp"

Refractive::Refractive(double opacity, double refractiveIndex, Vec3 color)
    : _opacity(opacity), _refractiveIndex(refractiveIndex), _color(color) {}

Vec3 Refractive::shade([[maybe_unused]] const HitRecord& record,
                        [[maybe_unused]] const Vec3& lightDir,
                        [[maybe_unused]] const Vec3& lightColor,
                        [[maybe_unused]] const Vec3& viewDir) const {
    // Refractive materials don't contribute to direct lighting
    return Vec3(0, 0, 0);
}

bool Refractive::scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation,
                          Ray& scattered) const {
    attenuation = _color / 255.0;

    double refractionRatio = record.front_face ? (1.0 / _refractiveIndex) : _refractiveIndex;

    Vec3 unitDirection = normalize(ray_in.direction());

    double cosTheta = std::min(dot(-unitDirection, record.normal), 1.0);
    double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

    bool cannotRefract = refractionRatio * sinTheta > 1.0;

    Vec3 direction;

    // Decide whether to reflect or refract
    if (cannotRefract) {
        // Total internal reflection
        direction = reflect(unitDirection, record.normal);
    } else {
        // For now, always refract when possible (can add Fresnel probability later)
        direction = refract(unitDirection, record.normal, refractionRatio);
    }

    scattered = Ray(record.point + direction * 1e-4, direction);
    return true;
}

Vec3 Refractive::shadowTransmittance() const {
    Vec3 normalizedColor = _color / 255.0;
    return normalizedColor * (1.0 - _opacity);
}

Vec3 Refractive::reflect(const Vec3& v, const Vec3& n) const { return v - 2.0 * dot(v, n) * n; }

// Refraction (Snell's Law in vector form)
Vec3 Refractive::refract(const Vec3& uv, const Vec3& n, double etai_over_etat) const {
    double cosTheta = std::min(dot(-uv, n), 1.0);
    Vec3 rOutPerp = etai_over_etat * (uv + cosTheta * n);
    Vec3 rOutParallel = -std::sqrt(std::abs(1.0 - length_squared(rOutPerp))) * n;
    return rOutPerp + rOutParallel;
}

extern "C" IMaterial* create(
    double opacity,
    double refractiveIndex,
    Vec3C color
) {
    return new Refractive(
        opacity,
        refractiveIndex,
        Vec3(color)
    );
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "refractive",
        .pluralForm = "refractive",
        .helpText = "Refractive (name, opacity, refractiveIndex, color (r, g, b))",
        .category = "material"
    };
    return &meta;
}