#pragma once

#include <memory>
#include "../../Interfaces/IMaterial.hpp"
#include "../../core/Image.hpp"

class NormalMapped : public IMaterial {
public:
    NormalMapped(const std::string& path, Vec3 albedo);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
               const Vec3& viewDir) const override;
    bool scatter(const Ray& ray_in, const HitRecord& record,
                 Vec3& attenuation, Ray& scattered) const override;
    Vec3 shadowTransmittance() const override;

private:
    std::unique_ptr<Image> _normalMap;
    Vec3 _albedo;
};
