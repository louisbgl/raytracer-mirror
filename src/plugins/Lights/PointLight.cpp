#include "PointLight.hpp"
#include "../PluginMetadata.hpp"

PointLight::PointLight(Vec3 position, Vec3 color, double intensity)
    : _position(position), _color(color), _intensity(intensity) {}

double PointLight::get_light_data(const Vec3& hit_point, Vec3& direction, Vec3& color) const {
    direction = normalize(_position - hit_point);
    double distance = length(_position - hit_point);
    color = (_color * _intensity) / (distance * distance);
    return distance;
}

extern "C" ILight* create(
    Vec3C position,
    Vec3C color,
    double intensity
) {
    return new PointLight(
        Vec3(position),
        Vec3(color),
        intensity
    );
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "pointlight",
        .pluralForm = "point",
        .helpText = "PointLight (position (x, y, z), color (r, g, b), intensity)",
        .category = "light"
    };
    return &meta;
}