#include "Triangle.hpp"
#include "../PluginMetadata.hpp"

extern "C" IShape* create(
    double v0x, double v0y, double v0z,
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z,
    std::shared_ptr<IMaterial>* material
) {
    return new Triangle(
        Vec3(v0x, v0y, v0z),
        Vec3(v1x, v1y, v1z),
        Vec3(v2x, v2y, v2z),
        *material
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "triangle",
        .pluralForm = "triangles",
        .helpText = "Triangle (v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z, material): flat-shaded triangle. Per-vertex normals and UVs available via direct construction (ObjParser/Mesh), not through this plugin interface.",
        .category = "shape"
    };
    return &metadata;
}
