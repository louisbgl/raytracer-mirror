#pragma once

#include <span>
#include <random>
#include "DataTypes/Vec3.hpp"

class RenderSampler {
public:
    RenderSampler() = delete;

    static double computeVariance(std::span<const Vec3> colors);
    static Vec3   randomHemisphere(const Vec3& normal);
    static Vec3   toneMapACES(Vec3 color, double strength = 1.0);

private:
    thread_local static std::mt19937 _gen;
};
