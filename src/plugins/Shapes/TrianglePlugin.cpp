#include "Triangle.hpp"
#include "../PluginMetadata.hpp"

extern "C" IShape* create(
    double rx, double ry, double rz,
    double tx, double ty, double tz,
    double v0x, double v0y, double v0z,
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z,
    std::shared_ptr<IMaterial>* material
) {
    return new Triangle(
        Vec3(v0x, v0y, v0z),
        Vec3(v1x, v1y, v1z),
        Vec3(v2x, v2y, v2z),
        Vec3(rx, ry, rz),
        Vec3(tx, ty, tz),
        *material
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "triangle",
        .pluralForm = "triangles",
        .helpText = "Triangle (v0 (x, y, z), v1 (x, y, z), v2 (x, y, z), material, [position (x, y, z)], [rotation (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}
