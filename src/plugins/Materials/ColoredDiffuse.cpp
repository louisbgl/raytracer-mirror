#include "ColoredDiffuse.hpp"
#include "../PluginMetadata.hpp"

#include "../../DataTypes/HitRecord.hpp"

ColoredDiffuse::ColoredDiffuse(Vec3 albedo) : _albedo(albedo / 255.0) {}

Vec3 ColoredDiffuse::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
                           [[maybe_unused]] const Vec3& viewDir) const {
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return (_albedo * (lightColor / 255.0)) * brightness * 255.0;
}

bool ColoredDiffuse::scatter([[maybe_unused]] const Ray& ray_in,
                             [[maybe_unused]] const HitRecord& record,
                             [[maybe_unused]] Vec3& attenuation,
                             [[maybe_unused]] Ray& scattered) const {
    return false;
}

Vec3 ColoredDiffuse::shadowTransmittance() const {
    return Vec3(0, 0, 0);
}

extern "C" IMaterial* create(Vec3C color) {
    return new ColoredDiffuse(Vec3(color));
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "coloreddiffuse",
        .pluralForm = "coloreddiffuse",
        .helpText = "ColoredDiffuse (name, color (r, g, b))",
        .category = "material"
    };
    return &meta;
}
