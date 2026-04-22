#include "ColoredDiffuse.hpp"
#include "../../DataTypes/HitRecord.hpp"

ColoredDiffuse::ColoredDiffuse(Vec3 albedo) : _albedo(albedo / 255.0) {}

Vec3 ColoredDiffuse::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor) const {
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return (_albedo * (lightColor / 255.0)) * brightness * 255.0;
}

bool ColoredDiffuse::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                             [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

extern "C" IMaterial* create(double r, double g, double b) {
    return new ColoredDiffuse(Vec3(r, g, b));
}
