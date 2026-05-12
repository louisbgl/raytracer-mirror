#pragma once

#include "core/ARayMarchedShape.hpp"

class MengerSponge : public ARayMarchedShape {
public:
    MengerSponge(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
        int maxSteps = 100, double epsilon = 1e-3, double maxDist = 100.0, int iterations = 5);
    ~MengerSponge() override = default;

private:
    int _iterations;

    double distanceEstimator(const Vec3& point) const override;
    AABB computeLocalAABB() const override;
};