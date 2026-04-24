
#include "./PerlinNoise.hpp"
#include "DataTypes/Ray.hpp"
#include "DataTypes/Vec3.hpp"
#include "../../DataTypes/HitRecord.hpp"

#include <cmath>
#include <cstdlib>


PerlinNoise::PerlinNoise(Vec3 albedo, double scale)
{
    fillArray();
    _scale = scale;
    _albedo = albedo;
}

void PerlinNoise::fillArray()
{
    for (int i = 0; i < PerlinNoise::POINT_COUNT; i++) {
        double rx = (2.0 * rand() / RAND_MAX) - 1.0;
        double ry = (2.0 * rand() / RAND_MAX) - 1.0;
        double rz = (2.0 * rand() / RAND_MAX) - 1.0;
        _ranVec3[i] = normalize(Vec3(rx, ry, rz)); 
    }

    perlinGenPerm(_permX);
    perlinGenPerm(_permY);
    perlinGenPerm(_permZ);
}

double PerlinNoise::fractal_noise(Vec3 p) const
{
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    for (int i = 0; i < 4; i++) { // 4 Octaves
        total += noise(p * frequency) * amplitude;
        amplitude *= 0.5; // Lower layers are dimmer
        frequency *= 2.0; // Lower layers are more detailed
    }
    return total;
}

Vec3 lerp(const Vec3& a, const Vec3& b, double t) {
    return a * (1.0 - t) + b * t;
}

bool PerlinNoise::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                            [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const
{
    return false;
}

Vec3 PerlinNoise::shade(const HitRecord& record, [[maybe_unused]]const Vec3& lightDir, [[maybe_unused]]const Vec3& lightColor, [[maybe_unused]] const Vec3& viewDir) const 
{
    double noise_val = fractal_noise(record.point * _scale);
    
    // Convert noise from [-1, 1] to [0, 1]
    double intensity = 0.5 * (1.0 + noise_val);
    
    // Simple color ramp for Nebula: Deep Blue to Bright Purple
    Vec3 nebula_color = lerp(Vec3(0.05, 0.0, 0.1), Vec3(0.5, 0.0, 0.8), intensity);
    
    // High-frequency star noise
    double star_noise = noise(record.point * 200.0);
    Vec3 stars = (star_noise > 0.98) ? Vec3(1, 1, 1) : Vec3(0, 0, 0);

    // If you want a "Space Vibe", ignore traditional lighting (diff) so it glows
    return (nebula_color * _albedo) + stars;
} 

double PerlinNoise::noise(const Vec3& point) const
{
    int i = static_cast<int>(std::floor(point.x()));
    int j = static_cast<int>(std::floor(point.y()));
    int k = static_cast<int>(std::floor(point.z()));

    double u = point.x() - std::floor(point.x());
    double v = point.y() - std::floor(point.y());
    double w = point.z() - std::floor(point.z());

    double u_sqrd = u * u * (3 - 2 * u);
    double v_sqrd = v * v * (3 - 2 * v);
    double w_sqrd = w * w * (3 - 2 * w);

    CubeVectors c;

    for (int di = 0; di < 2; di++) {
        for (int dj = 0; dj < 2; dj++) {
            for (int dk = 0; dk < 2; dk++) {

                // & 255 is a quick % 256
                // XOR mixes our indices
                c[di][dj][dk] = _ranVec3[
                    _permX[(i + di) & 255] ^
                    _permY[(j + dj) & 255] ^
                    _permZ[(k + dk) & 255]
                ];
            }
        }
    }

    return trilinearInterp(c, u_sqrd, v_sqrd, w_sqrd);
}

void PerlinNoise::perlinGenPerm(std::array<int, POINT_COUNT>& p)
{
    for (int i = 0; i < POINT_COUNT; i++) {
        p[i] = i;
    }

    for (int i = POINT_COUNT - 1; i > 0; i--) {
        int target = rand() % (i + 1);
        std::swap(p[i], p[target]);
    }
}

double PerlinNoise::trilinearInterp(const CubeVectors& c, double u, double v, double w)
{
    double accum = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                double weight = (i * u + (1 - i) * (1 - u)) *
                                  (j * v + (1 - j) * (1 - v)) *
                                  (k * w + (1 - k) * (1 - w));
                accum += weight * dot(c[i][j][k], Vec3(u - i, v - j, w - k));
            }
        }
    }

    return accum;
}

extern "C" IMaterial* create(double r, double g, double b, double scale) {
    return new PerlinNoise(Vec3(r, g, b), scale);
}
