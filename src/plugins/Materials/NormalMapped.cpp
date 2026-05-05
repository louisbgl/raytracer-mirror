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
    // TODO
    return Vec3(0, 0, 0);
}
