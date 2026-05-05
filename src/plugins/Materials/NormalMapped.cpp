#include "NormalMapped.hpp"
#include "../PluginMetadata.hpp"
#include "../../DataTypes/HitRecord.hpp"
#include <stdexcept>

NormalMapped::NormalMapped(const std::string& path, Vec3 albedo)
    : _albedo(albedo / 255.0)
{
    _normalMap = Image::readFile(path);
    if (!_normalMap)
        throw std::runtime_error("NormalMapped: cannot load " + path);
}

bool NormalMapped::scatter(const Ray&, const HitRecord&, Vec3&, Ray&) const
{
    return false;
}

Vec3 NormalMapped::shadowTransmittance() const
{
    return Vec3(0, 0, 0);
}

Vec3 NormalMapped::shade(const HitRecord& record, const Vec3& lightDir, const Vec3& lightColor,
                         [[maybe_unused]] const Vec3& viewDir) const
{
    Vec3 pixel = _normalMap->sample(record.u, record.v);
    Vec3 tangentNormal = (pixel / 255.0) * 2.0 - Vec3(1.0, 1.0, 1.0);

    Vec3 n = record.normal;
    Vec3 up = (std::abs(n.y()) < 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 tangent = normalize(up - n * dot(up, n));
    Vec3 bitangent = cross(n, tangent);

    Vec3 perturbedNormal = normalize(
        tangent * tangentNormal.x() +
        bitangent * tangentNormal.y() +
        n * tangentNormal.z()
    );

    double brightness = std::max(0.0, dot(perturbedNormal, lightDir));
    return (_albedo * (lightColor / 255.0)) * brightness * 255.0;
}

extern "C" IMaterial* create(const char* path, Vec3C albedo)
{
    return new NormalMapped(std::string(path), Vec3(albedo));
}

extern "C" const PluginMetadata* metadata()
{
    static PluginMetadata meta = {
        .pluginName = "normalmapped",
        .pluralForm = "normalmapped",
        .helpText = "NormalMapped (name, path)",
        .category = "material"
    };
    return &meta;
}
