#include "Chessboard.hpp"
#include "../PluginMetadata.hpp"

#include "../../DataTypes/HitRecord.hpp"

Chessboard::Chessboard(Vec3 color1, Vec3 color2, double scale)
    : _color1(color1 / 255.0), _color2(color2 / 255.0), _scale(scale) {}

Vec3 Chessboard::shade(const HitRecord& record, const Vec3& lightDir, [[maybe_unused]] const Vec3& lightColor, [[maybe_unused]] const Vec3& viewDir) const {

    // TODO: switch to UV coordinates (record.u, record.v) once #87 lands
    // Bias avoids floor() flipping sign on near-zero coordinates (floating point hit point noise)
    constexpr double eps = 1e-5;
    int check = static_cast<int>(std::floor(record.point.x() * _scale + eps)) +
                static_cast<int>(std::floor(record.point.y() * _scale + eps)) +
                static_cast<int>(std::floor(record.point.z() * _scale + eps));
    Vec3 baseColor = (check % 2 == 0) ? _color1 : _color2;
    double brightness = std::max(0.0, dot(record.normal, lightDir));
    return baseColor * brightness * 255.0;
}

bool Chessboard::scatter([[maybe_unused]] const Ray& ray_in, [[maybe_unused]] const HitRecord& record,
                            [[maybe_unused]] Vec3& attenuation, [[maybe_unused]] Ray& scattered) const {
    return false;
}

extern "C" IMaterial* create(double r1, double g1, double b1, double r2, double g2, double b2, double scale) {
    return new Chessboard(Vec3(r1, g1, b1), Vec3(r2, g2, b2), scale);
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