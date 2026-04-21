#include "PointLight.hpp"

PointLight::PointLight(Vec3 position, Vec3 color) : _position(position), _color(color) {}

double PointLight::get_light_data(const Vec3& hit_point, Vec3& direction, Vec3& color) const {
    direction = normalize(_position - hit_point);
    double distance = length(_position - hit_point);
    color = _color / (distance * distance);
    return distance;
}

extern "C" ILight* create(double x, double y, double z, double r, double g, double b) {
    return new PointLight(Vec3(x, y, z), Vec3(r, g, b));
}