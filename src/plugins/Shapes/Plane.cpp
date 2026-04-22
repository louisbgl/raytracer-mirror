#include "Plane.hpp"

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
    return true;
}

extern "C" IShape* create(
    double x,  double y,  double z,
    double nx, double ny, double nz,
    std::shared_ptr<IMaterial>* material) {
    return new Plane(Vec3(x, y, z), Vec3(nx, ny, nz), *material);
}
