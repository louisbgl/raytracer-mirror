#pragma once

#include <atomic>

struct RenderStats {
    std::atomic<long long> aoRaysCast{0};
    std::atomic<long long> aoRaysHit{0};

    std::atomic<long long> ssaaSamples{0};

    std::atomic<long long> adaptiveSamples{0};
    std::atomic<long long> adaptiveMaxSamples{0};

    std::atomic<long long> shadowRaysCast{0};
    std::atomic<long long> shadowRaysHit{0};

    std::atomic<long long> scatterBounces{0};

    RenderStats() = default;
    RenderStats(const RenderStats&) = delete;
    RenderStats& operator=(const RenderStats&) = delete;
};
