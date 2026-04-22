#pragma once

#include "../../Interfaces/IMaterial.hpp"

class ColoredDiffuse : public IMaterial {
public:
    ColoredDiffuse(Vec3 albedo);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
               [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                 [[maybe_unused]] Vec3& attenuation,
                 [[maybe_unused]] Ray& scattered) const override;

private:
    Vec3 _albedo;
};
