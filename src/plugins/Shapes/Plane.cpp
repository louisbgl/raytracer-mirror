#include "Plane.hpp"
#include "../PluginMetadata.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>

Plane::Plane(Vec3 pos, Vec3 normal, std::shared_ptr<IMaterial> material)
    : _pos(pos), _normal(normal), _material(material) {}

bool Plane::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    double denom = dot(ray.direction(), _normal);
    if (std::abs(denom) < 1e-6) return false;

    double t = dot(_pos - ray.origin(), _normal) / denom;

    if (t < t_min || t > t_max) return false;

    record.t = t;
    record.point = ray.at(t);
    record.material = _material;
    record.set_face_normal(ray, _normal);

    Vec3 up_hint = (std::abs(_normal.y()) < 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 tangent = normalize(cross(up_hint, _normal));
    Vec3 bitangent = cross(_normal, tangent);
    Vec3 local = record.point - _pos;
    record.u = dot(local, tangent);
    record.v = dot(local, bitangent);

    return true;
}

extern "C" IShape* create(
    Vec3C position,
    Vec3C normal,
    std::shared_ptr<IMaterial>* material
) {
    return new Plane(
        Vec3(position),
        Vec3(normal),
        *material
    );
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "plane",
        .pluralForm = "planes",
        .helpText = "Plane (position (x, y, z), normal (x, y, z), material)",
        .category = "shape"
    };
    return &metadata;
}