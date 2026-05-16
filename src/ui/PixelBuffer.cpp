#include "PixelBuffer.hpp"

void PixelBuffer::init(int w, int h) {
    std::lock_guard<std::mutex> lock(mutex);
    width = w;
    height = h;
    rgba.resize(w * h * 4, 0);
    rowsComplete.store(0);
    totalRows.store(0);
    cancel.store(false);
    done.store(false);
}

void PixelBuffer::reset() {
    std::lock_guard<std::mutex> lock(mutex);
    rgba.clear();
    width = 0;
    height = 0;
    rowsComplete.store(0);
    totalRows.store(0);
    cancel.store(false);
    done.store(false);
}

void PixelBuffer::setRow(int y, const uint8_t* rowRgba, int rowBytes) {
    std::lock_guard<std::mutex> lock(mutex);
    if (y < 0 || y >= height || rowBytes < width * 4) {
        return;
    }
    std::copy(rowRgba, rowRgba + width * 4, rgba.begin() + y * width * 4);
}

void PixelBuffer::swapBuffers(PixelBuffer& other) {
    std::swap(rgba, other.rgba);
    std::swap(width, other.width);
    std::swap(height, other.height);

    int tempRows = rowsComplete.load();
    rowsComplete.store(other.rowsComplete.load());
    other.rowsComplete.store(tempRows);

    int tempTotal = totalRows.load();
    totalRows.store(other.totalRows.load());
    other.totalRows.store(tempTotal);
}
