
#include "./Cone.hpp"
#include "DataTypes/Vec3.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include <cmath>



static constexpr double EPS = 1e-9;

Cone::Cone(Vec3 pos, double radius, std::shared_ptr<IMaterial>material)
{
    _position = pos;
    _radius = radius;
    _material = material;
}

bool Cone::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const
{
    Vec3 oc = ray.origin() - _position;
    double k2 = _radius * _radius;

    double a = (ray.direction().x() * ray.direction().x()) + (ray.direction().z() * ray.direction().z()) - (k2 * ray.direction().y() * ray.direction().y());
    double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z() - k2 * oc.y() * ray.direction().y());
    double c = (oc.x() * oc.x()) + (oc.z() * oc.z()) - (k2 * oc.y() * oc.y());

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
        // Gonna assume the "bottom" part of the hourglass for the basic cone
        if (hitpoint.y() > _position.y() ) {
            continue;
        }
        double rel_x = hitpoint.x() - _position.x();
        double rel_z = hitpoint.z() - _position.z();
        double rel_y = hitpoint.y() - _position.y();

    //    double cone_r = std::sqrt(rel_x * rel_x + rel_z * rel_z);
        Vec3 outward_norm = Vec3(rel_x, -k2 * rel_y, rel_z);
        outward_norm = normalize(outward_norm);

        record.t = t;
        record.point = hitpoint;
        record.material = _material;
        record.set_face_normal(ray, outward_norm);

        return true;
    }
    return false;
}

extern "C" IShape* create(double x, double y, double z, double radius, std::shared_ptr<IMaterial>* material) {
    return new Cone(Vec3(x, y, z), radius, *material);
}

