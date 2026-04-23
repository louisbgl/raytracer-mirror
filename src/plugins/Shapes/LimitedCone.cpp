#include "LimitedCone.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include <cmath>
#include <limits>

LimitedCone::LimitedCone(Vec3 rotation, Vec3 translation, double radius, double height, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _radius(radius), _height(height), _material(material) {}

bool LimitedCone::hitLocal(const Ray& ray, HitRecord& record) const {
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
            double y_min = -_height;
            double y_max = 0.0;

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

                double cone_r = std::sqrt((rel_x * rel_x) + (rel_z * rel_z));
                double norm_y = (rel_y > 0) ? -cone_r * k : cone_r * k;
                Vec3 outward_norm = Vec3(rel_x, norm_y, rel_z);
                outward_norm = normalize(outward_norm);

                record.t = t;
                record.point = hitpoint;
                record.material = _material;
                record.set_face_normal(ray, outward_norm);

                closest_t = t;
                hit_anything = true;
            }
        }
    }

    hit_anything |= checkBaseIntersection(ray, closest_t, record);

    return hit_anything;
}

bool LimitedCone::checkBaseIntersection(const Ray& ray, double& closest_t, HitRecord& record) const {
    if (std::abs(ray.direction().y()) < 1e-8) {
        return false;
    }

    double base_y = -_height;
    double t = (base_y - ray.origin().y()) / ray.direction().y();

    if (t < 0 || t >= closest_t) {
        return false;
    }

    Vec3 hit_point = ray.at(t);
    double dist_from_center = std::sqrt(hit_point.x() * hit_point.x() + hit_point.z() * hit_point.z());

    double base_radius = _radius * _height;
    if (dist_from_center > base_radius) {
        return false;
    }

    record.t = t;
    record.point = hit_point;
    record.material = _material;
    record.set_face_normal(ray, Vec3(0, -1, 0));

    return true;
}

AABB LimitedCone::computeLocalAABB() const {
    double base_radius = _radius * _height;
    Vec3 min(-base_radius, -_height, -base_radius);
    Vec3 max(base_radius, 0, base_radius);
    return AABB(min, max);
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double radius, double height, std::shared_ptr<IMaterial>* material) {
    return new LimitedCone(Vec3(rx, ry, rz), Vec3(tx, ty, tz), radius, height, *material);
}