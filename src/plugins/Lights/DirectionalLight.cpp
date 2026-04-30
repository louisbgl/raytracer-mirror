#include "DirectionalLight.hpp"
#include "../PluginMetadata.hpp"

DirectionalLight::DirectionalLight(Vec3 direction, Vec3 color, double intensity)
    : _direction(direction), _color(color), _intensity(intensity) {}

double DirectionalLight::get_light_data([[maybe_unused]] const Vec3& hit_point, Vec3& direction,
                                        Vec3& color) const {
    direction = normalize(-_direction);
    color = _color * _intensity;
    return 1e9;
}

extern "C" ILight* create(
    Vec3C direction,
    Vec3C color,
    double intensity
) {
    return new DirectionalLight(
        Vec3(direction),
        Vec3(color),
        intensity
    );
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "directionallight",
        .pluralForm = "directional",
        .helpText = "DirectionalLight (direction (dx, dy, dz), color (r, g, b), intensity)",
        .category = "light"
    };
    return &meta;
}