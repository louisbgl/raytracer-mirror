#include "LimitedHourglass.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>
#include <limits>

LimitedHourglass::LimitedHourglass(Vec3 rotation, Vec3 translation, double radius, double height, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _radius(radius), _height(height), _material(material) {}

bool LimitedHourglass::hitLocal(const Ray& ray, HitRecord& record) const {
    double closest_t = std::numeric_limits<double>::infinity();
    bool hit_anything = false;

    Vec3 oc = ray.origin();
    double k = _radius;
    double k_sqrd = k * k;

    double a = (ray.direction().x() * ray.direction().x()) + (ray.direction().z() * ray.direction().z()) - (k_sqrd * ray.direction().y() * ray.direction().y());

    if (std::abs(a) >= 1e-9) {
        double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z() - k_sqrd * oc.y() * ray.direction().y());
        double c = (oc.x() * oc.x()) + (oc.z() * oc.z()) - (k_sqrd * oc.y() * oc.y());

        QuadraticRoots roots = QuadraticSolver::solve(a, b, c);
        if (roots.hasRoots()) {
            double half_h = _height / 2.0;
            double y_min = -half_h;
            double y_max = half_h;

            for (int i = 0; i < roots.count; i++) {
                double t = roots[i];
                if (t < 0 || t >= closest_t) {
                    continue;
                }

                Vec3 hitpoint = ray.at(t);
                if (hitpoint.y() < y_min || hitpoint.y() > y_max) {
                    continue;
                }

                double rel_x = hitpoint.x();
                double rel_z = hitpoint.z();
                double rel_y = hitpoint.y();
                double norm_y = -k_sqrd * rel_y;
                Vec3 outward_norm = normalize(Vec3(rel_x, norm_y, rel_z));

                record.t = t;
                record.point = hitpoint;
                record.material = _material;
                record.set_face_normal(ray, outward_norm);
                record.u = std::atan2(hitpoint.z(), hitpoint.x()) * Math::INV_TWO_PI + 0.5;
                record.v = (hitpoint.y() + _height / 2.0) / _height;

                closest_t = t;
                hit_anything = true;
            }
        }
    }

    double half_h = _height / 2.0;
    hit_anything |= checkCapIntersection(ray, -half_h, closest_t, record);
    hit_anything |= checkCapIntersection(ray, half_h, closest_t, record);

    return hit_anything;
}

bool LimitedHourglass::checkCapIntersection(const Ray& ray, double cap_y, double& closest_t, HitRecord& record) const {
    if (std::abs(ray.direction().y()) < 1e-8) {
        return false;
    }

    double t = (cap_y - ray.origin().y()) / ray.direction().y();

    if (t < 0 || t >= closest_t) {
        return false;
    }

    Vec3 hit_point = ray.at(t);
    double dist_from_center = std::sqrt(hit_point.x() * hit_point.x() + hit_point.z() * hit_point.z());

    double cap_radius = _radius * std::abs(cap_y);
    if (dist_from_center > cap_radius) {
        return false;
    }

    Vec3 normal = (cap_y > 0) ? Vec3(0, 1, 0) : Vec3(0, -1, 0);

    record.t = t;
    record.point = hit_point;
    record.material = _material;
    record.set_face_normal(ray, normal);
    if (cap_radius > 1e-9) {
        record.u = hit_point.x() / (2.0 * cap_radius) + 0.5;
        record.v = hit_point.z() / (2.0 * cap_radius) + 0.5;
    } else {
        record.u = 0.5;
        record.v = 0.5;
    }

    closest_t = t;
    return true;
}

AABB LimitedHourglass::computeLocalAABB() const {
    double half_h = _height / 2.0;
    double max_radius = _radius * half_h;
    Vec3 min(-max_radius, -half_h, -max_radius);
    Vec3 max(max_radius, half_h, max_radius);
    return AABB(min, max);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double radius, double height, std::shared_ptr<IMaterial>* material) {
    return new LimitedHourglass(Vec3(rx, ry, rz), Vec3(tx, ty, tz), radius, height, *material);
}
