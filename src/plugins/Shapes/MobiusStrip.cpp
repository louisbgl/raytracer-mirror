#include "MobiusStrip.hpp"
#include "../PluginMetadata.hpp"

MobiusStrip::MobiusStrip(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                         double radius, double width, double thickness, int twists,
                         int maxSteps, double epsilon, double maxDist)
    : ARayMarchedShape(rotation, translation, scale, material, maxSteps, epsilon, maxDist),
      _radius(radius), _width(width), _thickness(thickness), _twists(twists) {}

double MobiusStrip::distanceEstimator(const Vec3& point) const {
    double angle = std::atan2(point.z(), point.x());
    double r = std::sqrt(point.x() * point.x() + point.z() * point.z());
    double y = point.y();

    double twist_angle = angle * _twists / 2.0;
    double cos_t = std::cos(twist_angle);
    double sin_t = std::sin(twist_angle);
    double u = (r - _radius) * cos_t + y * sin_t;
    double v = -(r - _radius) * sin_t + y * cos_t;

    // double dist_radial = std::abs(r - _radius) - _width; // unused for now
    double dist_width = std::abs(u) - _width * 0.5;
    double dist_thickness = std::abs(v) - _thickness * 0.5;

    return std::max(dist_width, dist_thickness);
}

AABB MobiusStrip::computeLocalAABB() const {
    double outer = _radius + _width;
    double half_thickness = _thickness * 0.5;

    return AABB(
        Vec3(-outer, -half_thickness, -outer),
        Vec3(outer, half_thickness, outer)
    );
}

extern "C" IShape* create(
    Vec3C rotation,
    Vec3C translation,
    Vec3C scale,
    double radius,
    double width,
    double thickness,
    int twists,
    std::shared_ptr<IMaterial>* material
) {
    return new MobiusStrip(
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
        *material,
        radius,
        width,
        thickness,
        twists,
        100,     // maxSteps
        0.001,   // epsilon
        100.0    // maxDist
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "mobius_strip",
        .pluralForm = "mobius_strips",
        .helpText = "Mobius Strip (position (x, y, z), radius, width, thickness, twists, material, [rotation (x, y, z)], [scale (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}
