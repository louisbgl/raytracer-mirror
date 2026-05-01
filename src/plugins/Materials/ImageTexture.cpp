#include "ImageTexture.hpp"
#include "../PluginMetadata.hpp"
#include "../../DataTypes/HitRecord.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>

ImageTexture::ImageTexture(const std::string& filename) {
    _image = Image::readFile(filename);
    if (!_image)
        throw std::runtime_error("ImageTexture: cannot load " + filename);
}

Vec3 ImageTexture::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
                         [[maybe_unused]] const Vec3& viewDir) const {
    Vec3 color = _image->sample(record.u, record.v);

    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return (color / 255.0) * (lightColor / 255.0) * brightness * 255.0;
}

bool ImageTexture::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                           [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

Vec3 ImageTexture::shadowTransmittance() const {
    return Vec3(0, 0, 0);
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