#include "MengerSponge.hpp"
#include "../PluginMetadata.hpp"

MengerSponge::MengerSponge(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                             int maxSteps, double epsilon, double maxDist, int iterations)
    : ARayMarchedShape(rotation, translation, scale, material, maxSteps, epsilon, maxDist), _iterations(iterations) {}

double MengerSponge::distanceEstimator(const Vec3& point) const {
    Vec3 q = abs(point) - Vec3(1, 1, 1);
    double d = length(max(q, 0)) + std::min(max_component(q), 0.0);
    
    double s = 1.0;
    for (int i = 0; i < _iterations; ++i) {
        Vec3 a = fmod(point * s, 2.0) - Vec3(1, 1, 1);
        s *= 3.0;

        Vec3 r = Vec3(1, 1, 1) - 3.0 * abs(a);
        double cx = std::max(r.x(), r.y());
        double cy = std::max(r.y(), r.z());
        double cz = std::max(r.z(), r.x());
        double cross_dist = std::min({cx, cy, cz}) / s;

        d = std::max(d, -cross_dist);
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
    std::shared_ptr<IMaterial>* material,
    int maxSteps,
    double epsilon,
    double maxDist,
    int iterations
) {
    return new MengerSponge(
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
        *material,
        maxSteps,
        epsilon,
        maxDist,
        iterations
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "menger_sponge",
        .pluralForm = "menger_sponges",
        .helpText = "Menger Sponge (position (x, y, z), material, iterations, [rotation (x, y, z)], [scale (x, y, z)], [maxSteps], [epsilon], [maxDist])",
        .category = "shape"
    };
    return &metadata;
}