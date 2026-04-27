#pragma once

#include <string>
#include "Vec3.hpp"

struct RendererConfig {
    bool aaEnabled = false;
    int aaSamples = 1;
    std::string aaMethod = "ssaa";

    double ambientMultiplier = 0.4;
    double diffuseMultiplier = 0.6;

    Vec3 ambientColor = Vec3(25, 25, 38);
    Vec3 backgroundColor = Vec3(135, 206, 235);
    std::string backgroundImage = ""; // If set, this will override backgroundColor

    std::string outputFile = "output.ppm";
};