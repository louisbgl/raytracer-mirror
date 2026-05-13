#include "Triangle.hpp"
#include "../PluginMetadata.hpp"

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation, scale, material),
      _v0(v0),
      _edge1(v1 - v0),
      _edge2(v2 - v0),
      _normal(normalize(cross(_edge1, _edge2)))
{
}

bool Triangle::hitLocal(const Ray& ray, HitRecord& record) const {
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

    record.t = t;
    record.point = ray.at(t);
    record.material = _material;
    record.u = u;
    record.v = v;
    record.set_face_normal(ray, _normal);

    return true;
}

AABB Triangle::computeLocalAABB() const {
    Vec3 v1 = _v0 + _edge1;
    Vec3 v2 = _v0 + _edge2;
    return AABB(
        Vec3(
            std::min({ _v0.x(), v1.x(), v2.x() }) - 1e-4,
            std::min({ _v0.y(), v1.y(), v2.y() }) - 1e-4,
            std::min({ _v0.z(), v1.z(), v2.z() }) - 1e-4
        ),
        Vec3(
            std::max({ _v0.x(), v1.x(), v2.x() }) + 1e-4,
            std::max({ _v0.y(), v1.y(), v2.y() }) + 1e-4,
            std::max({ _v0.z(), v1.z(), v2.z() }) + 1e-4
        )
    );
}

extern "C" IShape* create(
    Vec3C rotation,
    Vec3C translation,
    Vec3C scale,
    Vec3C v0,
    Vec3C v1,
    Vec3C v2,
    std::shared_ptr<IMaterial>* material
) {
    return new Triangle(
        Vec3(v0),
        Vec3(v1),
        Vec3(v2),
        Vec3(rotation),
        Vec3(translation),
        Vec3(scale),
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
