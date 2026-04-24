#include "Sphere.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>
#include <algorithm>

Sphere::Sphere(Vec3 rotation, Vec3 translation, double radius, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _radius(radius), _material(material) {}

bool Sphere::hitLocal(const Ray& ray, HitRecord& record) const {
    Vec3 oc = ray.origin();
    double a = dot(ray.direction(), ray.direction());
    double b = 2.0 * dot(oc, ray.direction());
    double c = dot(oc, oc) - _radius * _radius;

    auto roots = QuadraticSolver::solve(a, b, c);
    if (!roots.hasRoots()) {
        return false;
    }

    double t = roots.t1;
    if (t < 0) {
        t = roots.t2;
        if (t < 0) {
            return false;
        }
    }

    Vec3 hit_point = ray.at(t);
    Vec3 outward_normal = normalize(hit_point);

    record.point = hit_point;
    record.t = t;
    record.material = _material;
    record.set_face_normal(ray, outward_normal);

    double phi = std::atan2(-outward_normal.z(), outward_normal.x()) + Math::PI;
    double cos_theta = std::clamp(-outward_normal.y(), -1.0, 1.0);
    record.u = phi * Math::INV_TWO_PI;
    record.v = std::acos(cos_theta) * Math::INV_PI;

    return true;
}

AABB Sphere::computeLocalAABB() const {
    Vec3 min(-_radius, -_radius, -_radius);
    Vec3 max(_radius, _radius, _radius);
    return AABB(min, max);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double radius, std::shared_ptr<IMaterial>* material) {
    return new Sphere(Vec3(rx, ry, rz), Vec3(tx, ty, tz), radius, *material);
}