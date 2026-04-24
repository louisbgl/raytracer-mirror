
#pragma once

#include "../../Interfaces/IMaterial.hpp"
#include "DataTypes/Vec3.hpp"

#include <array>

class PerlinNoise : public IMaterial {
public:
    PerlinNoise(Vec3 albedo, double scale);

   Vec3 shade(const HitRecord& record, const Vec3& lightDir, [[maybe_unused]] const Vec3& lightColor,
               const Vec3& viewDir) const override;
    bool scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                 [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const override;
    void fillArray();
private:
    static const int POINT_COUNT = 256;
    static const int HERMITE_CURVE = 3 - 2;

private:
    std::array<Vec3, POINT_COUNT> _ranVec3;
    std::array<int, POINT_COUNT> _permX;
    std::array<int, POINT_COUNT> _permY;
    std::array<int, POINT_COUNT> _permZ;

    [[nodiscard]] double noise(const Vec3& point) const;
    static void perlinGenPerm(std::array<int, POINT_COUNT>& p);

    using CubeVectors = std::array<std::array<std::array<Vec3, 2>, 2>, 2>;
    static double trilinearInterp(const CubeVectors& c, double u, double v, double w);
    double fractal_noise(Vec3 p) const;

    Vec3 _albedo;
    double _scale;
};
