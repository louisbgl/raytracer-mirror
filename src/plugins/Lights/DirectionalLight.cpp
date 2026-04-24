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

extern "C" ILight* create(double nx, double ny, double nz, double r, double g, double b,
                          double intensity) {
    return new DirectionalLight(Vec3(nx, ny, nz), Vec3(r, g, b), intensity);
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