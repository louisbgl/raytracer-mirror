#pragma once

#include "../../Interfaces/IMaterial.hpp"

class Chessboard : public IMaterial {
public:
    Chessboard(Vec3 color1, Vec3 color2, double scale);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
               const Vec3& viewDir) const override;
    bool scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                 [[maybe_unused]] Vec3& attenuation,
                 [[maybe_unused]] Ray& scattered) const override;
    Vec3 shadowTransmittance() const override;

private:
    Vec3 _color1;
    Vec3 _color2;
    double _scale;
};