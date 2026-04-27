#include "Mesh.hpp"
#include "../../parsers/ObjParser.hpp"
#include "../PluginMetadata.hpp"

Mesh::Mesh(const std::string& filename, std::shared_ptr<IMaterial> material) {
    ObjParser parser;
    auto shapes = parser.parse(filename, material);
    if (shapes.empty())
        throw std::runtime_error("Mesh: no triangles loaded from " + filename);
    _bvh = std::make_shared<BVHNode>(shapes, 0, shapes.size());
    _aabb = _bvh->boundingBox();
}

bool Mesh::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    return _bvh->hit(ray, t_min, t_max, record);
}

AABB Mesh::boundingBox() const {
    return _aabb;
}

extern "C" IShape* create(const char* filename, std::shared_ptr<IMaterial>* material) {
    return new Mesh(std::string(filename), *material);
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "mesh",
        .pluralForm = "meshes",
        .helpText = "Mesh (filename, material)",
        .category = "shape"
    };
    return &metadata;
}