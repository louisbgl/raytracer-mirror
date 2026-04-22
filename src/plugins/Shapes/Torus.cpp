#include "Torus.hpp"
#include "../../Math/QuarticSolver.hpp"
#include <algorithm>
#include <cmath>

Torus::Torus(Vec3 position, double majorRadius, double minorRadius, std::shared_ptr<IMaterial> material)
    : _position(std::move(position)), _majorRadius(majorRadius), _minorRadius(minorRadius), _material(material) {}

bool Torus::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    Vec3 oc = ray.origin() - _position;
    Vec3 d = ray.direction();

    double sum_d_sqr = dot(d, d);
    double sum_oc_sqr = dot(oc, oc);
    double e_oc_d = dot(oc, d);
    double R2 = _majorRadius * _majorRadius;
    double r2 = _minorRadius * _minorRadius;

    double a = sum_d_sqr * sum_d_sqr;
    double b = 4 * sum_d_sqr * e_oc_d;
    double c = 2 * sum_d_sqr * (sum_oc_sqr - (R2 + r2)) + 4 * e_oc_d * e_oc_d + 4 * R2 * (d.y() * d.y());
    double d_coeff = 4.0 * (sum_oc_sqr - (R2 + r2)) * e_oc_d + 8.0 * R2 * (oc.y() * d.y());
    double e = (sum_oc_sqr - (R2 + r2)) * (sum_oc_sqr - (R2 + r2)) - 4 * R2 * (r2 - oc.y() * oc.y());

    QuarticRoots roots = QuarticSolver::solve(e, d_coeff, c, b, a);
    if (!roots.hasRoots()) return false;

    double closest_t = t_max;
    bool hit_anything = false;

    for (int i = 0; i < roots.count; ++i) {
        double t = roots.roots[i];

        if (t > t_min && t < closest_t) {
            closest_t = t;
            hit_anything = true;
        }
    }

    if (!hit_anything) return false;

    record.t = closest_t;
    record.point = ray.at(record.t);
    record.set_face_normal(ray, computeNormal(record.point));
    record.material = _material;
    return true;
}

Vec3 Torus::computeNormal(const Vec3& point) const {
    Vec3 p = point - _position;

    double sum_sqr = dot(p, p);
    double k = sum_sqr - _majorRadius * _majorRadius - _minorRadius * _minorRadius;

    Vec3 normal(
        4.0 * p.x() * k,
        4.0 * p.y() * (k + 2.0 * _majorRadius * _majorRadius),
        4.0 * p.z() * k
    );

    return normalize(normal);
}

extern "C" IShape* create(double x, double y, double z, double majorRadius, double minorRadius, std::shared_ptr<IMaterial>* material) {
    return new Torus(Vec3(x, y, z), majorRadius, minorRadius, *material);
}