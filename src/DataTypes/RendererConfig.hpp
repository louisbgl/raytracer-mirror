#pragma once

#include <string>

struct RendererConfig {
    bool aaEnabled = false;
    int aaSamples = 1;
    std::string aaMethod = "ssaa";
};
