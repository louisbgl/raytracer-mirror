#pragma once

#include <vector>
#include <cstdint>

#include "../../Interfaces/IMaterial.hpp"

class ImageTexture : public IMaterial {
public:
    explicit ImageTexture(const std::string& filename);

    Vec3 shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
               [[maybe_unused]] const Vec3& viewDir) const override;
    bool scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                 [[maybe_unused]] Vec3& attenuation,
                 [[maybe_unused]] Ray& scattered) const override;

private:
    int _width;
    int _height;
    std::vector<uint8_t> _data;

    
};