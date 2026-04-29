#include "Mesh.hpp"
#include "../../parsers/ObjParser.hpp"
#include "../PluginMetadata.hpp"
#include <stdexcept>

Mesh::Mesh(const std::string& filename, Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation, scale)
{
    ObjParser parser;
    auto shapes = parser.parse(filename, material);
    if (shapes.empty())
        throw std::runtime_error("Mesh: no triangles loaded from " + filename);
    _bvh = std::make_shared<BVHNode>(shapes, 0, shapes.size());
}

bool Mesh::hitLocal(const Ray& localRay, HitRecord& record) const {
    return _bvh->hit(localRay, 0, std::numeric_limits<double>::infinity(), record);
}

AABB Mesh::computeLocalAABB() const {
    return _bvh->boundingBox();
}

extern "C" IShape* create(const char* filename, double rx, double ry, double rz, double tx, double ty, double tz, double sx, double sy, double sz, std::shared_ptr<IMaterial>* material) {
    return new Mesh(std::string(filename), Vec3(rx, ry, rz), Vec3(tx, ty, tz), Vec3(sx, sy, sz), *material);
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "mesh",
        .pluralForm = "meshes",
        .helpText = "Mesh (path, position (x, y, z), material, [rotation (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}
