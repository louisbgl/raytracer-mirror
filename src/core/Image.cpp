#include "Image.hpp"
#include <fstream>
#include <algorithm>

void Image::setPixel(int x, int y, const Vec3& color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return; // Out of bounds, do nothing
    _pixels[y * _width + x] = color;
}

void Image::writePPM(const std::string& filename) const {
    std::ofstream ofs(filename);
    ofs << "P3\n" << _width << " " << _height << "\n255\n";
    for (const auto& pixel : _pixels) {
        int r = static_cast<int>(std::clamp(pixel.x(), 0.0, 255.0));
        int g = static_cast<int>(std::clamp(pixel.y(), 0.0, 255.0));
        int b = static_cast<int>(std::clamp(pixel.z(), 0.0, 255.0));
        ofs << r << " " << g << " " << b << "\n";
    }
    ofs.close();
}