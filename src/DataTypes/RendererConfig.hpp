#pragma once

#include <string>
#include "Vec3.hpp"

struct RendererConfig {
    bool aaEnabled = false;
    int aaSamples = 1; // Only for SSAA
    double aaThreshold = 0.1; // Only for adaptive SSAA
    std::string aaMethod = "ssaa"; // "ssaa" | "adaptive"

    double ambientMultiplier = 0.4;
    double diffuseMultiplier = 0.6;

    Vec3 ambientColor = Vec3(25, 25, 38);
    Vec3 backgroundColor = Vec3(135, 206, 235);
    std::string backgroundImage = ""; // If set, this will override backgroundColor

    std::string outputFile = "output.ppm";

    bool multithreadingEnabled = true;
    int threadCount = 0; // 0 means auto-detect based on hardware concurrency
};
