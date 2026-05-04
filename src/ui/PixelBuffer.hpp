#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <mutex>
#include <vector>

// Shared between render thread (write) and UI thread (read).
struct PixelBuffer {
    std::mutex             mutex;
    std::vector<sf::Uint8> rgba;        // w * h * 4, RGBA, row-major, top-down
    int                    width  = 0;
    int                    height = 0;
    std::atomic<int>       rowsComplete{0};
    std::atomic<int>       totalRows{0};    // set by Core after scene parse
    std::atomic<bool>      cancel{false};
    std::atomic<bool>      done{false};

    void init(int w, int h);            // allocate + zero, reset all atomics
    void reset();                       // zero pixels + atomics, keep allocation
    void setRow(int y, const sf::Uint8* rowRgba, int rowBytes); // thread-safe write
};
