#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

struct PixelBuffer {
    mutable std::mutex     mutex;
    std::vector<uint8_t>   rgba;        // w * h * 4, RGBA, row-major, top-down
    int                    width  = 0;
    int                    height = 0;
    std::atomic<int>       rowsComplete{0};
    std::atomic<int>       totalRows{0};
    std::atomic<bool>      cancel{false};
    std::atomic<bool>      done{false};

    void init(int w, int h);
    void reset();
    void setRow(int y, const uint8_t* rowRgba, int rowBytes); // thread safe write
    void swapBuffers(PixelBuffer& other); // swap all members (mutexes stay in place)
};
