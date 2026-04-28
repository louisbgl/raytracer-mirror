#include "MeshTriangle.hpp"

MeshTriangle::MeshTriangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<IMaterial> material,
                           std::optional<std::array<Vec3, 3>> normals,
                           std::optional<std::array<UV, 3>> uvs)
    : _v0(v0),
      _edge1(v1 - v0),
      _edge2(v2 - v0),
      _normal(normalize(cross(_edge1, _edge2))),
      _material(material),
      _vertexNormals(normals),
      _vertexUVs(uvs)
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

bool MeshTriangle::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
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

    double w = 1.0 - u - v;

    record.t = t;
    record.point = ray.at(t);
    record.material = _material;

    if (_vertexNormals) {
        const auto& n = *_vertexNormals;
        record.set_face_normal(ray, normalize(w * n[0] + u * n[1] + v * n[2]));
    } else {
        record.set_face_normal(ray, _normal);
    }

    if (_vertexUVs) {
        const auto& uv = *_vertexUVs;
        record.u = w * uv[0].u + u * uv[1].u + v * uv[2].u;
        record.v = w * uv[0].v + u * uv[1].v + v * uv[2].v;
    } else {
        record.u = u;
        record.v = v;
    }

    return true;
}
