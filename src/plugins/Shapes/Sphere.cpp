#include "Sphere.hpp"

Sphere::Sphere(Vec3 pos, double radius, std::shared_ptr<IMaterial> material) : _position(pos), _radius(radius), _material(material) {}

bool Sphere::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    Vec3 oc = ray.origin() - _position;
    double a = dot(ray.direction(), ray.direction());
    double b = 2.0 * dot(oc, ray.direction());
    double c = dot(oc, oc) - _radius * _radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    }

    double sqrt_discriminant = std::sqrt(discriminant);
    double t = (-b - sqrt_discriminant) / (2.0 * a);

    if (t < t_min || t > t_max) {
        t = (-b + sqrt_discriminant) / (2.0 * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }

    Vec3 hit_point = ray.at(t);
    Vec3 outward_normal = normalize(hit_point - _position);

    record.point = hit_point;
    record.t = t;
    record.material = _material;
    record.set_face_normal(ray, outward_normal);

    return true;
}