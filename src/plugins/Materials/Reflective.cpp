#include "Reflective.hpp"
#include "../PluginMetadata.hpp"
#include "../../DataTypes/HitRecord.hpp"
#include <algorithm>

Reflective::Reflective(double reflectivity, Vec3 color)
    : _reflectivity(_clampReflectivity(reflectivity)), _color(color) {}

Vec3 Reflective::shade([[maybe_unused]] const HitRecord& record,
                        [[maybe_unused]] const Vec3& lightDir,
                        [[maybe_unused]] const Vec3& lightColor,
                        [[maybe_unused]] const Vec3& viewDir) const {
    // Reflective materials don't contribute to direct lighting
    return Vec3(0, 0, 0);
}

bool Reflective::scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const {
    Vec3 reflectedDir = ray_in.direction() - record.normal * 2.0 * dot(ray_in.direction(), record.normal);
    reflectedDir = normalize(reflectedDir);
    scattered = Ray(record.point + record.normal * 1e-4, reflectedDir);
    attenuation = (_color / 255.0) * _reflectivity;
    return true;
}

Vec3 Reflective::shadowTransmittance() const {
    return Vec3(0, 0, 0);
}

double Reflective::_clampReflectivity(double reflectivity) {
    return std::clamp(reflectivity, 0.0, 1.0);
}

extern "C" IMaterial* create(double reflectivity, double r, double g, double b) {
    return new Reflective(reflectivity, Vec3(r, g, b));
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "reflective",
        .pluralForm = "reflective",
        .helpText = "Reflective (name, reflectivity, color (r, g, b))",
        .category = "material"
    };
    return &meta;
}