#include "Lambertian.hpp"
#include "../../DataTypes/HitRecord.hpp"

Lambertian::Lambertian(Vec3 albedo) : _albedo(albedo) {}

Vec3 Lambertian::shade(const HitRecord& record, const Vec3& lightDir, [[maybe_unused]] const Vec3& lightColor) const {
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return _albedo * brightness;
}

bool Lambertian::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                         [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

extern "C" IMaterial* create(double r, double g, double b) {
    return new Lambertian(Vec3(r, g, b));
}