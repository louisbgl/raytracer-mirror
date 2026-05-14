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

    // Twist angle (half-twists)
    double twist_angle = angle * _twists * 0.5;
    double cos_t = std::cos(twist_angle);
    double sin_t = std::sin(twist_angle);

    // Rotate (r - radius, y) to get strip-local coords
    double u = (r - _radius) * cos_t - y * sin_t;
    double v = (r - _radius) * sin_t + y * cos_t;

    // Distance to strip bounds
    double dist_width = std::abs(u) - _width * 0.5;
    double dist_thickness = std::abs(v) - _thickness * 0.5;

    // Box SDF in (u,v) space
    Vec3 q(std::max(dist_width, 0.0), std::max(dist_thickness, 0.0), 0.0);
    double dist = length(q) + std::min(std::max(dist_width, dist_thickness), 0.0);

    // Scale by Lipschitz bound (compensate for twist stretching)
    // Higher twists = more stretching = need conservative bound
    double lipschitz = 1.0 + std::abs(_twists) * 0.5 / _radius;
    return dist / lipschitz;
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
