#include "Triangle.hpp"
#include "../PluginMetadata.hpp"

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<IMaterial> material)
    : _v0(v0),
      _edge1(v1 - v0),
      _edge2(v2 - v0),
      _normal(normalize(cross(_edge1, _edge2))),
      _material(material)
      {
    Vec3 min(
        std::min({ v0.x(), v1.x(), v2.x() }) - 1e-4,
        std::min({ v0.y(), v1.y(), v2.y() }) - 1e-4,
        std::min({ v0.z(), v1.z(), v2.z() }) - 1e-4
    );
    Vec3 max(
        std::max({ v0.x(), v1.x(), v2.x() }) + 1e-4,
        std::max({ v0.y(), v1.y(), v2.y() }) + 1e-4,
        std::max({ v0.z(), v1.z(), v2.z() }) + 1e-4
    );
    _aabb = AABB(min, max);
}

bool Triangle::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    if (!_aabb.hit(ray, t_min, t_max)) return false;

    Vec3 h = cross(ray.direction(), _edge2);
    double a = dot(_edge1, h);

    if (std::abs(a) < _epsilon) return false;

    double f = 1.0 / a;
    Vec3 s = ray.origin() - _v0;
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    Vec3 q = cross(s, _edge1);
    double v = f * dot(ray.direction(), q);
    if (v < 0.0 || u + v > 1.0) return false;

    double t = f * dot(_edge2, q);
    if (t < t_min || t > t_max) return false;

    record.t = t;
    record.point = ray.at(t);
    record.u = u;
    record.v = v;
    record.set_face_normal(ray, _normal);
    record.material = _material;

    return true;
}

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
        .helpText = "Triangle (v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z, material): A triangle defined by three vertices and a material.",
        .category = "shape"
    };
    return &metadata;
}