#pragma once

#include <chrono>
#include <string>

class ProgressBar {
public:
    explicit ProgressBar(int total);

    void update(int current);
    double finish();

private:
    static constexpr int BAR_WIDTH = 30;

    int _total;
    std::chrono::steady_clock::time_point _start;

    static std::string _formatTime(double seconds);
};
