#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <atomic>

class ProgressBar {
public:
    explicit ProgressBar(int total);

    void update(int delta = 1);
    double finish();

private:
    static constexpr int BAR_WIDTH = 20;

    int _total;
    std::chrono::steady_clock::time_point _start;

    static std::string _formatTime(double seconds);

    std::atomic<int> _currentThreads{0};
    std::mutex _dispMutex;
};
