#include "RenderSampler.hpp"

#include <cmath>
#include <thread>
#include <functional>

thread_local std::mt19937 RenderSampler::_gen(
    std::hash<std::thread::id>{}(std::this_thread::get_id())
);

double RenderSampler::computeVariance(std::span<const Vec3> colors) {
    Vec3 mean(0, 0, 0);
    for (const auto& c : colors)
        mean += c;
    mean /= static_cast<double>(colors.size());

    double variance = 0.0;
    for (const auto& c : colors) {
        Vec3 diff = c - mean;
        variance += dot(diff, diff);
    }
    return variance / static_cast<double>(colors.size());
}

Vec3 RenderSampler::randomHemisphere(const Vec3& normal) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double r1 = dist(_gen);
    double r2 = dist(_gen);
    double sinTheta = std::sqrt(1.0 - r1 * r1);
    double phi = 2.0 * M_PI * r2;

    double x = sinTheta * std::cos(phi);
    double y = sinTheta * std::sin(phi);
    double z = r1;

    // Build ONB around normal
    Vec3 up = std::abs(normal.x()) > 0.9 ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 tangent   = normalize(cross(up, normal));
    Vec3 bitangent = cross(normal, tangent);

    return normalize(tangent * x + bitangent * y + normal * z);
}
