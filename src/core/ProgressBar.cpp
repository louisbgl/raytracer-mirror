#include "ProgressBar.hpp"

#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

static std::string repeat(const std::string& s, int n) {
    std::string out;
    out.reserve(s.size() * static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

ProgressBar::ProgressBar(int total)
    : _total(total), _start(std::chrono::steady_clock::now()) {
    std::cout << std::endl;
}

void ProgressBar::update(int delta) {
    if (_total <= 0) return;

    int current = _currentThreads.fetch_add(delta) + delta;
    if (current % 10 != 0 && current != _total) {
        return;
    }

    std::lock_guard<std::mutex> lock(_dispMutex);

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - _start).count();

    float pct = static_cast<float>(current) / _total;
    if (pct > 1.0f) {
        pct = 1.0f;
    }
    int filled = static_cast<int>(pct * BAR_WIDTH);

    std::string bar = repeat("█", filled) + repeat("░", BAR_WIDTH - filled);

    std::string eta = (elapsed > 0.5 && current > 0)
        ? _formatTime((_total - current) * elapsed / current)
        : "--";

    std::cout << "\033[2K\r"
              << "  Rendering  [" << bar << "]  "
              << std::setw(3) << static_cast<int>(pct * 100) << "%"
              << "  " << current << "/" << _total << " rows"
              << "  ETA " << eta
              << std::flush;
}

double ProgressBar::finish() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - _start).count();

    std::string bar = repeat("█", BAR_WIDTH);
    std::cout << "\033[2K\r"
              << "  Rendering  [" << bar << "] 100%"
              << "  " << _total << "/" << _total << " rows"
              << "  done in " << _formatTime(elapsed)
              << std::endl;
    return elapsed;
}

std::string ProgressBar::_formatTime(double s) {
    std::ostringstream ss;
    if (s < 1.0)
        ss << static_cast<int>(s * 1000) << "ms";
    else if (s < 60.0)
        ss << std::fixed << std::setprecision(1) << s << "s";
    else
        ss << static_cast<int>(s) / 60 << "m" << static_cast<int>(s) % 60 << "s";
    return ss.str();
}
