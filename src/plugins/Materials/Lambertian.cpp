#include "Lambertian.hpp"
#include "../PluginMetadata.hpp"

#include "../../DataTypes/HitRecord.hpp"

Lambertian::Lambertian(Vec3 albedo) : _albedo(albedo / 255.0) {}

Vec3 Lambertian::shade(const HitRecord& record, const Vec3& lightDir,
                       [[maybe_unused]] const Vec3& lightColor,
                       [[maybe_unused]] const Vec3& viewDir) const {
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return _albedo * brightness * 255.0;
}

bool Lambertian::scatter([[maybe_unused]] const Ray& ray_in,
                         [[maybe_unused]] const HitRecord& record,
                         [[maybe_unused]] Vec3& attenuation,
                         [[maybe_unused]] Ray& scattered) const {
    return false;
}

Vec3 Lambertian::shadowTransmittance() const {
    return Vec3(0, 0, 0);
}

extern "C" IMaterial* create(double r, double g, double b) { return new Lambertian(Vec3(r, g, b)); }

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "lambertian",
        .pluralForm = "lambertian",
        .helpText = "Lambertian (name, color (r, g, b))",
        .category = "material"
    };
    return &meta;
}
