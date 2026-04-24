#include "./Cone.hpp"
#include "DataTypes/Vec3.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>

static constexpr double EPS = 1e-9;

Cone::Cone(Vec3 pos, Vec3 axis, double radius, std::shared_ptr<IMaterial> material)
    : _position(pos), _axis(normalize(axis)), _radius(radius), _material(material) {}

bool Cone::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
{
    Vec3 oc = ray.origin() - _position;
    double k2 = _radius * _radius;

    double dot_d_axis = dot(ray.direction(), _axis);
    double dot_oc_axis = dot(oc, _axis);

    Vec3 d_perp = ray.direction() - _axis * dot_d_axis;
    Vec3 oc_perp = oc - _axis * dot_oc_axis;

    double a = dot(d_perp, d_perp) - k2 * dot_d_axis * dot_d_axis;
    double b = 2.0 * (dot(oc_perp, d_perp) - k2 * dot_oc_axis * dot_d_axis);
    double c = dot(oc_perp, oc_perp) - k2 * dot_oc_axis * dot_oc_axis;

    if (std::abs(a) < EPS) {
        return false;
    }

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

        if (along_axis > 0) {
            continue;
        }

        Vec3 p_perp = p - _axis * along_axis;
        Vec3 outward_norm = normalize(p_perp - _axis * k2 * along_axis);

        record.t = t;
        record.point = hitpoint;
        record.material = _material;
        record.set_face_normal(ray, outward_norm);

        double perp_len = length(p_perp);
        if (perp_len > 1e-9) {
            Vec3 ref = (std::abs(_axis.x()) < 0.9) ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
            Vec3 tangent = normalize(cross(ref, _axis));
            Vec3 pn = p_perp / perp_len;
            record.u = std::atan2(dot(pn, cross(_axis, tangent)), dot(pn, tangent)) * Math::INV_TWO_PI + 0.5;
        }
        record.v = -along_axis;

        return true;
    }
    return false;
}

extern "C" IShape* create(double x, double y, double z, double ax, double ay, double az, double radius, std::shared_ptr<IMaterial>* material) {
    return new Cone(Vec3(x, y, z), Vec3(ax, ay, az), radius, *material);
}

