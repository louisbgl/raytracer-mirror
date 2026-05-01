#include "Image.hpp"

#include "external/stb_image.h"
#include "external/stb_image_write.h"

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

void Image::writeFile(const std::string& filename) const {
    std::string ext = filename.substr(filename.find_last_of('.') + 1);

    auto buffer = _getPixelBuffer();
    bool success = false;
    if (ext == "png") {
        success = stbi_write_png(filename.c_str(), _width, _height, 3, buffer.data(), _width * 3);
    } else if (ext == "jpg" || ext == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), _width, _height, 3, buffer.data(), 90);
    } else if (ext == "ppm") {
        _writePPM(filename, true);
        success = true;
    } else {
        std::string fallback = filename.substr(0, filename.find_last_of('.')) + ".ppm";
        std::cerr << "Image::writeFile: unrecognized extension '" << ext << "', defaulting to binary PPM (P6)" << std::endl;
        _writePPM(fallback, true);
        success = true;
        if (success) std::cout << "Image saved to " << fallback << std::endl;
        return;
    }

    if (success) std::cout << "Image saved to " << filename << std::endl;
    else         std::cerr << "Failed to save image to " << filename << std::endl;

}

std::unique_ptr<Image> Image::readFile(const std::string& filename) {
    std::string ext = filename.substr(filename.find_last_of('.') + 1);

    if (ext == "ppm") return _readPPM(filename);

    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if (!data) {
        std::cerr << "Failed to load image: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    std::unique_ptr<Image> image = std::make_unique<Image>(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            image->setPixel(x, y, Vec3(data[idx], data[idx + 1], data[idx + 2]));
        }
    }

    stbi_image_free(data);
    return image;
}

std::vector<unsigned char> Image::_getPixelBuffer() const {
    std::vector<unsigned char> buffer(_width * _height * 3);
    for (size_t i = 0; i < _pixels.size(); ++i) {
        const Vec3& pixel = _pixels[i];
        buffer[i * 3 + 0] = static_cast<unsigned char>(std::clamp(pixel.x(), 0.0, 255.0));
        buffer[i * 3 + 1] = static_cast<unsigned char>(std::clamp(pixel.y(), 0.0, 255.0));
        buffer[i * 3 + 2] = static_cast<unsigned char>(std::clamp(pixel.z(), 0.0, 255.0));
    }
    return buffer;
}

void Image::_writePPMp3(const std::string& filename) const {
    std::ofstream ofs(filename);
    ofs << "P3\n" << _width << " " << _height << "\n255\n";

    std::ostringstream row_buffer;
    for (int y = 0; y < _height; ++y) {
        row_buffer.str("");
        row_buffer.clear();

        for (int x = 0; x < _width; ++x) {
            const Vec3& pixel = _pixels[y * _width + x];
            int r = static_cast<int>(std::clamp(pixel.x(), 0.0, 255.0));
            int g = static_cast<int>(std::clamp(pixel.y(), 0.0, 255.0));
            int b = static_cast<int>(std::clamp(pixel.z(), 0.0, 255.0));
            row_buffer << r << " " << g << " " << b << "\n";
        }

        ofs << row_buffer.str();
    }
    ofs.close();
}

void Image::_writePPMp6(const std::string& filename) const {
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << _width << " " << _height << "\n255\n";

    std::vector<unsigned char> row_buffer(_width * 3);

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            const Vec3& pixel = _pixels[y * _width + x];
            row_buffer[x * 3 + 0] = static_cast<unsigned char>(std::clamp(pixel.x(), 0.0, 255.0));
            row_buffer[x * 3 + 1] = static_cast<unsigned char>(std::clamp(pixel.y(), 0.0, 255.0));
            row_buffer[x * 3 + 2] = static_cast<unsigned char>(std::clamp(pixel.z(), 0.0, 255.0));
        }
        ofs.write(reinterpret_cast<const char*>(row_buffer.data()), row_buffer.size());
    }
    ofs.close();
}

void Image::_writePPM(const std::string& filename, bool binary) const {
    if (binary) _writePPMp6(filename);
    else        _writePPMp3(filename);
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

std::unique_ptr<Image> Image::_readPPM(const std::string& filename) {
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