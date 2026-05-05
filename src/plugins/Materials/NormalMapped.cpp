#include "NormalMapped.hpp"
#include "../PluginMetadata.hpp"
#include "../../DataTypes/HitRecord.hpp"
#include <stdexcept>

NormalMapped::NormalMapped(const std::string& path)
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

    // TODO
    return Vec3(0, 0, 0);
}
