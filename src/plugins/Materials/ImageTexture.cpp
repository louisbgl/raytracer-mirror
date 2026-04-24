#include "ImageTexture.hpp"
#include "../PluginMetadata.hpp"
#include "../../DataTypes/HitRecord.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

ImageTexture::ImageTexture(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("ImageTexture: cannot open " + filename);

    std::string magic;
    file >> magic;
    if (magic != "P6" && magic != "P3")
        throw std::runtime_error("ImageTexture: unsupported PPM format (only P3/P6)");

    char c;
    file.get(c);
    while (file.peek() == '#') {
        std::string line;
        std::getline(file, line);
    }

    int maxval;
    file >> _width >> _height >> maxval;
    file.get(c);

    _data.resize(_width * _height * 3);

    if (magic == "P6") {
        file.read(reinterpret_cast<char*>(_data.data()), _data.size());
    } else {
        for (size_t i = 0; i < _data.size(); i++) {
            int v;
            file >> v;
            _data[i] = static_cast<uint8_t>(v);
        }
    }
}

Vec3 ImageTexture::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
                         [[maybe_unused]] const Vec3& viewDir) const {
    int x = static_cast<int>(record.u * _width) % _width;
    int y = static_cast<int>((1.0 - record.v) * _height) % _height;
    if (x < 0) x += _width;
    if (y < 0) y += _height;

    int idx = (y * _width + x) * 3;
    Vec3 color(_data[idx], _data[idx + 1], _data[idx + 2]);

    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return (color / 255.0) * (lightColor / 255.0) * brightness * 255.0;
}

bool ImageTexture::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                           [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

extern "C" IMaterial* create(const char* path) {
    return new ImageTexture(std::string(path));
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "image_texture",
        .pluralForm = "image_textures",
        .helpText = "ImageTexture (name, path)",
        .category = "material"
    };
    return &meta;
}