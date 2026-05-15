#include "JuliaSet3D.hpp"
#include "../PluginMetadata.hpp"

JuliaSet3D::JuliaSet3D(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                         Vec3 c, double power, int iterations, double bailout)
    : ARayMarchedShape(rotation, translation, scale, material),
      _c(c), _power(power), _iterations(iterations), _bailout(bailout) {}

double JuliaSet3D::distanceEstimator(const Vec3& point) const {
    Vec3 z = point;
    double dr = 1.0;
    
    for (int i = 0; i < _iterations; ++i) {
        double r = length(z);
        if (r > _bailout) break;

        // To spherical coords
        double theta = std::acos(std::clamp(z.z() / r, -1.0, 1.0));
        double phi = std::atan2(z.y(), z.x());

        // Derivative : dr = r^(power-1) * power * dr + 1
        dr = std::pow(r, _power - 1.0) * _power * dr + 1.0;
        
        // z = z^power + c in quaternion space
        double zr = std::pow(r, _power);
        theta *= _power;
        phi *= _power;
        
        // Back to cartesian coordinates
        double sinTheta = std::sin(theta);
        z = Vec3(
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            std::cos(theta)
        ) * zr + _c;
    }

    // Distance estimate: 0.5 * log(r) * r / dr
    double r = length(z);
    return 0.5 * std::log(r) * r / dr;
}

AABB JuliaSet3D::computeLocalAABB() const {
    double bound = _bailout + 0.5;
    return AABB(Vec3(-bound, -bound, -bound), Vec3(bound, bound, bound));
}

extern "C" IShape* create(
    Vec3C rotation,
    Vec3C translation,
    Vec3C scale,
    Vec3C c,
    double power,
    int iterations,
    double bailout,
    std::shared_ptr<IMaterial>* material
) {
    return new JuliaSet3D(
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
        *material,
        Vec3(c),
        power,
        iterations,
        bailout
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "julia_set_3d",
        .pluralForm = "julia_set_3ds",
        .helpText = "Julia Set 3D (position (x, y, z), c (x, y, z), power, iterations, bailout, material, [rotation (x, y, z)], [scale (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}