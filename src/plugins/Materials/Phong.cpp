#include "Phong.hpp"
#include "../PluginMetadata.hpp"

#include "../../DataTypes/HitRecord.hpp"

Phong::Phong(Vec3 albedo, double shininess) : _albedo(albedo / 255.0), _shininess(shininess) {}

// ambient + diffuse + specular = phong
// ambient is handled by core
Vec3 Phong::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor, const Vec3& viewDir) const {
    // diffuse
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    Vec3 diffuse = (_albedo * (lightColor / 255.0)) * brightness;

    // specular
    Vec3 negL = -lightDir;
    Vec3 R = negL - record.normal * 2.0 * dot(negL, record.normal);
    double specPow = std::pow(std::max(0.0, dot(R, viewDir)), _shininess);
    Vec3 specular = (lightColor / 255.0) * specPow;

    return (diffuse + specular) * 255.0;
}

bool Phong::scatter([[maybe_unused]] const Ray& ray_in,
                             [[maybe_unused]] const HitRecord& record,
                             [[maybe_unused]] Vec3& attenuation,
                             [[maybe_unused]] Ray& scattered) const {
    return false;
}

Vec3 Phong::shadowTransmittance() const {
    return Vec3(0, 0, 0);
}

extern "C" IMaterial* create(
    Vec3C color,
    double shininess
) {
    return new Phong(
        Vec3(color),
        shininess
    );
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "phong",
        .pluralForm = "phong",
        .helpText = "Phong (name, color (r, g, b), shininess)",
        .category = "material"
    };
    return &meta;
}