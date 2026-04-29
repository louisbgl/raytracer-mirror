#include "Image.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory>

void Image::setPixel(int x, int y, const Vec3& color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    _pixels[y * _width + x] = color;
}

Vec3 Image::getPixel(int x, int y) const {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return Vec3(0, 0, 0);
    return _pixels[y * _width + x];
}

Vec3 Image::sample(double u, double v) const {
    int x = static_cast<int>(u * _width) % _width;
    int y = static_cast<int>((1.0 - v) * _height) % _height;
    if (x < 0) x += _width;
    if (y < 0) y += _height;
    return getPixel(x, y);
}

void Image::writePPM(const std::string& filename, bool binary) const {
    if (binary) _writePPMp6(filename);
    else        _writePPMp3(filename);
}

std::unique_ptr<Image> Image::readPPM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Image::readPPM: cannot open " << filename << std::endl;
        return nullptr;
    }

    std::string magic;
    file >> magic;
    if (magic != "P6" && magic != "P3") {
        std::cerr << "Image::readPPM: unsupported PPM format (only P3/P6)" << std::endl;
        return nullptr;
    }

    char c;
    file.get(c);
    while (file.peek() == '#') {
        std::string line;
        std::getline(file, line);
    }

    int width, height, maxval;
    file >> width >> height >> maxval;
    file.get(c);

    auto image = std::make_unique<Image>(width, height);

    if (magic == "P6") _readPPMp6(width, height, file, image);
    else               _readPPMp3(width, height, file, image);

    return image;
}

void Image::_writePPMp3(const std::string& filename) const {
    std::ofstream ofs(filename);
    ofs << "P3\n" << _width << " " << _height << "\n255\n";

    std::ostringstream row_buffer;
    for (int y = 0; y < _height; ++y) {
        row_buffer.str("");  // Clear buffer
        row_buffer.clear();  // Clear state flags

        for (int x = 0; x < _width; ++x) {
            const Vec3& pixel = _pixels[y * _width + x];
            int r = static_cast<int>(std::clamp(pixel.x(), 0.0, 255.0));
            int g = static_cast<int>(std::clamp(pixel.y(), 0.0, 255.0));
            int b = static_cast<int>(std::clamp(pixel.z(), 0.0, 255.0));
            row_buffer << r << " " << g << " " << b << "\n";
        }

        ofs << row_buffer.str();  // Write entire row at once
    }
    ofs.close();
}

void Image::_writePPMp6(const std::string& filename) const {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << _width << " " << _height << "\n255\n";

    // Allocate buffer for one row
    std::vector<unsigned char> row_buffer(_width * 3);

    for (int y = 0; y < _height; ++y) {
        // Fill row buffer
        for (int x = 0; x < _width; ++x) {
            const Vec3& pixel = _pixels[y * _width + x];
            row_buffer[x * 3 + 0] = static_cast<unsigned char>(std::clamp(pixel.x(), 0.0, 255.0));
            row_buffer[x * 3 + 1] = static_cast<unsigned char>(std::clamp(pixel.y(), 0.0, 255.0));
            row_buffer[x * 3 + 2] = static_cast<unsigned char>(std::clamp(pixel.z(), 0.0, 255.0));
        }
        // Write entire row at once
        ofs.write(reinterpret_cast<const char*>(row_buffer.data()), row_buffer.size());
    }
    ofs.close();
}

void Image::_readPPMp3(int width, int height, std::ifstream& file, std::unique_ptr<Image>& image) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r, g, b;
            file >> r >> g >> b;
            image->setPixel(x, y, Vec3(r, g, b));
        }
    }
}

void Image::_readPPMp6(int width, int height, std::ifstream& file, std::unique_ptr<Image>& image) {
    std::vector<uint8_t> data(width * height * 3);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            image->setPixel(x, y, Vec3(data[idx], data[idx + 1], data[idx + 2]));
        }
    }
}