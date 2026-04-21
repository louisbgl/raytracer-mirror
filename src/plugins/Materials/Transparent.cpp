#include "Transparent.hpp"

Transparent::Transparent(double opacity, double refractiveIndex, Vec3 color)
    : _opacity(opacity), _refractiveIndex(refractiveIndex), _color(color) {}

Vec3 Transparent::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor) const {
    (void)record;
    (void)lightDir;
    (void)lightColor;
    return Vec3(200, 0, 0);
}

bool Transparent::scatter(const Ray& ray_in, const HitRecord& record, Vec3& attenuation, Ray& scattered) const {
    (void)ray_in;
    (void)record;
    (void)attenuation;
    (void)scattered;
    return false;
}

extern "C" IMaterial* create(double opacity, double refractiveIndex, double r, double g, double b) {
    return new Transparent(opacity, refractiveIndex, Vec3(r, g, b));
}