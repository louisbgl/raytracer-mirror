#pragma once

#include "core/ARayMarchedShape.hpp"

class MobiusStrip : public ARayMarchedShape {
public:
    MobiusStrip(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                double radius = 5.0, double width = 2.0, double thickness = 0.2, int twists = 1,
                int maxSteps = 100, double epsilon = 1e-3, double maxDist = 100.0);
    ~MobiusStrip() override = default;

private:
    double _radius;
    double _width;
    double _thickness;
    int    _twists;

    double distanceEstimator(const Vec3& point) const override;
    AABB computeLocalAABB() const override;
};