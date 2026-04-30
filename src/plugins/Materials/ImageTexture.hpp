#pragma once

#include <memory>
#include "../../Interfaces/IMaterial.hpp"
#include "../../core/Image.hpp"

class ImageTexture : public IMaterial {
public:
    explicit ImageTexture(const std::string& filename);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
               [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                 [[maybe_unused]] Vec3& attenuation,
                 [[maybe_unused]] Ray& scattered) const override;
    Vec3 shadowTransmittance() const override;

private:
    std::unique_ptr<Image> _image;
};