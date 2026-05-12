#include "MengerSponge.hpp"
#include "../PluginMetadata.hpp"

MengerSponge::MengerSponge(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                             int maxSteps, double epsilon, double maxDist, int iterations)
    : ARayMarchedShape(rotation, translation, scale, material, maxSteps, epsilon, maxDist), _iterations(iterations) {}

double MengerSponge::distanceEstimator(const Vec3& point) const {
    Vec3 q = abs(point) - Vec3(1.0, 1.0, 1.0);
    double d = length(max(q, 0.0)) + std::min(max_component(q), 0.0);

    if (_iterations == 0) return d;

    auto centeredMod = [](double val, double period) {
        return val - period * std::floor(val / period + 0.5);
    };

    double s = 1.0;
    for (int i = 0; i < _iterations; ++i) {
        Vec3 a(centeredMod(point.x() * s, 2.0),
               centeredMod(point.y() * s, 2.0),
               centeredMod(point.z() * s, 2.0));
        s *= 3.0;

        Vec3 r = abs(a * 3.0);
        double da = std::max(r.x(), r.y());
        double db = std::max(r.y(), r.z());
        double dc = std::max(r.z(), r.x());
        double c = (std::min(da, std::min(db, dc)) - 1.0) / s;

        d = std::max(d, -c);
    }

    return d;
}

AABB MengerSponge::computeLocalAABB() const {
    return AABB(Vec3(-1, -1, -1), Vec3(1, 1, 1));
}

extern "C" IShape* create(
    Vec3C rotation,
    Vec3C translation,
    Vec3C scale,
    int iterations,
    std::shared_ptr<IMaterial>* material
) {
    return new MengerSponge(
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
        *material,
        100,
        0.001,
        100.0,
        iterations
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "menger_sponge",
        .pluralForm = "menger_sponges",
        .helpText = "Menger Sponge (position (x, y, z), iterations, material, [rotation (x, y, z)], [scale (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}