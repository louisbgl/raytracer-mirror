#include "Torus.hpp"
#include "../../Math/QuarticSolver.hpp"
#include "../../Math/Constants.hpp"
#include <algorithm>
#include <cmath>

Torus::Torus(Vec3 rotation, Vec3 translation, double majorRadius, double minorRadius, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _majorRadius(majorRadius), _minorRadius(minorRadius), _material(material) {}

bool Torus::hitLocal(const Ray& ray, HitRecord& record) const {
    Vec3 oc = ray.origin();
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

    double closest_t = std::numeric_limits<double>::infinity();
    bool hit_anything = false;
    const double epsilon = 1e-6;

    for (int i = 0; i < roots.count; ++i) {
        double t = roots.roots[i];

        if (t > epsilon && t < closest_t) {
            closest_t = t;
            hit_anything = true;
        }
    }

    if (!hit_anything) return false;

    record.t = closest_t;
    record.point = ray.at(closest_t);
    record.material = _material;
    Vec3 outward_normal = computeNormal(record.point);
    record.normal = outward_normal;
    record.front_face = true;

    Vec3 p = record.point;
    double phi = std::atan2(p.z(), p.x());
    double theta = std::atan2(p.y(), std::sqrt(p.x() * p.x() + p.z() * p.z()) - _majorRadius);
    record.u = phi * Math::INV_TWO_PI + 0.5;
    record.v = theta * Math::INV_TWO_PI + 0.5;

    return true;
}

AABB Torus::computeLocalAABB() const {
    double outer = _majorRadius + _minorRadius;
    Vec3 min(-outer, -_minorRadius, -outer);
    Vec3 max(outer, _minorRadius, outer);
    return AABB(min, max);
}

Vec3 Torus::computeNormal(const Vec3& point) const {
    double sum_sqr = dot(point, point);
    double k = sum_sqr - _majorRadius * _majorRadius - _minorRadius * _minorRadius;

    Vec3 normal(
        4.0 * point.x() * k,
        4.0 * point.y() * (k + 2.0 * _majorRadius * _majorRadius),
        4.0 * point.z() * k
    );

    return normalize(normal);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double majorRadius, double minorRadius, std::shared_ptr<IMaterial>* material) {
    return new Torus(Vec3(rx, ry, rz), Vec3(tx, ty, tz), majorRadius, minorRadius, *material);
}