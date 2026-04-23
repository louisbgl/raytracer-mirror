#include "./Hourglass.hpp"
#include "DataTypes/Vec3.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include <cmath>

Hourglass::Hourglass(Vec3 pos, Vec3 axis, double radius, std::shared_ptr<IMaterial> material)
    : _position(pos), _axis(normalize(axis)), _radius(radius), _material(material) {}

bool Hourglass::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
{
    Vec3 oc = ray.origin() - _position;
    double k_sqrd = _radius * _radius;

    double dot_d_axis = dot(ray.direction(), _axis);
    double dot_oc_axis = dot(oc, _axis);

    Vec3 d_perp = ray.direction() - _axis * dot_d_axis;
    Vec3 oc_perp = oc - _axis * dot_oc_axis;

    double a = dot(d_perp, d_perp) - k_sqrd * dot_d_axis * dot_d_axis;
    if (std::abs(a) < 1e-9) {
        return false;
    }

    double b = 2.0 * (dot(oc_perp, d_perp) - k_sqrd * dot_oc_axis * dot_d_axis);
    double c = dot(oc_perp, oc_perp) - k_sqrd * dot_oc_axis * dot_oc_axis;

    QuadraticRoots roots = QuadraticSolver::solve(a, b, c);
    if (!roots.hasRoots()) {
        return false;
    }

    for (int i = 0; i < roots.count; i++) {
        double t = roots[i];
        if (t < t_min || t > t_max) {
            continue;
        }

        Vec3 hitpoint = ray.at(t);
        Vec3 p = hitpoint - _position;
        double along_axis = dot(p, _axis);

        Vec3 p_perp = p - _axis * along_axis;
        Vec3 outward_norm = normalize(p_perp - _axis * k_sqrd * along_axis);

        record.t = t;
        record.point = hitpoint;
        record.material = _material;
        record.set_face_normal(ray, outward_norm);

        return true;
    }

    return false;
}

extern "C" IShape* create(double x, double y, double z, double ax, double ay, double az, double radius, std::shared_ptr<IMaterial>* material) {
    return new Hourglass(Vec3(x, y, z), Vec3(ax, ay, az), radius, *material);
}
