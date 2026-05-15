#pragma once

#include "core/ARayMarchedShape.hpp"

class JuliaSet3D : public ARayMarchedShape {
public:
    JuliaSet3D(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
               Vec3 c = Vec3(-0.8, 0.156, 0.0), double power = 8.0, int iterations = 20, double bailout = 4.0);
    ~JuliaSet3D() override = default;

private:
    Vec3   _c;
    double _power;
    int    _iterations;
    double _bailout;

    double distanceEstimator(const Vec3& point) const override;
    AABB computeLocalAABB() const override;
};
