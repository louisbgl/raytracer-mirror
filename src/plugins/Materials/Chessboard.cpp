#include "Chessboard.hpp"
#include "../PluginMetadata.hpp"

#include "../../DataTypes/HitRecord.hpp"

Chessboard::Chessboard(Vec3 color1, Vec3 color2, double scale)
    : _color1(color1 / 255.0), _color2(color2 / 255.0), _scale(scale) {}

Vec3 Chessboard::shade(const HitRecord& record, const Vec3& lightDir, [[maybe_unused]] const Vec3& lightColor, [[maybe_unused]] const Vec3& viewDir) const {

    int check = static_cast<int>(std::floor(record.u * _scale)) +
                static_cast<int>(std::floor(record.v * _scale));
    Vec3 baseColor = (check % 2 == 0) ? _color1 : _color2;
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return baseColor * (lightColor / 255.0) * brightness * 255.0;
}

bool Chessboard::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                            [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

Vec3 Chessboard::shadowTransmittance() const {
    return Vec3(0, 0, 0);
}

extern "C" IMaterial* create(
    Vec3C color1,
    Vec3C color2,
    double scale
) {
    return new Chessboard(
        Vec3(color1),
        Vec3(color2),
        scale
    );
}

extern "C" const PluginMetadata* metadata() {
    static PluginMetadata meta = {
        .pluginName = "chessboard",
        .pluralForm = "chessboard",
        .helpText = "Chessboard (name, color1 (r, g, b), color2 (r, g, b), scale)",
        .category = "material"
    };
    return &meta;
}