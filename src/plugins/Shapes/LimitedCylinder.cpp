#include "LimitedCylinder.hpp"
#include "../../Math/QuadraticSolver.hpp"
#include "../PluginMetadata.hpp"
#include "../../Math/Constants.hpp"
#include <cmath>
#include <limits>

LimitedCylinder::LimitedCylinder(Vec3 rotation, Vec3 translation, double radius, double height, std::shared_ptr<IMaterial> material)
    : AShape(rotation, translation), _radius(radius), _height(height), _material(material) {}

bool LimitedCylinder::hitLocal(const Ray& ray, HitRecord& record) const {
    double closest_t = std::numeric_limits<double>::infinity();
    bool hit_anything = false;

    hit_anything |= checkBodyIntersection(ray, 0.0, closest_t, record);
    hit_anything |= checkCapIntersection(ray, 0.0, Vec3(0, -1, 0), 0.0, closest_t, record);
    hit_anything |= checkCapIntersection(ray, _height, Vec3(0, 1, 0), 0.0, closest_t, record);

    return hit_anything;
}

AABB LimitedCylinder::computeLocalAABB() const {
    Vec3 min(-_radius, 0, -_radius);
    Vec3 max(_radius, _height, _radius);
    return AABB(min, max);
}

bool LimitedCylinder::checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const {
    Vec3 oc = ray.origin();

    double a = ray.direction().x() * ray.direction().x() + ray.direction().z() * ray.direction().z();
    double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z());
    double c = oc.x() * oc.x() + oc.z() * oc.z() - _radius * _radius;

    QuadraticRoots roots = QuadraticSolver::solve(a, b, c);
    if (!roots.hasRoots()) return false;

    bool found_hit = false;
    for (double t : {roots.t1, roots.t2}) {
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            double y = hit_point.y();

            if (isWithinHeight(y)) {
                record.t = t;
                record.point = hit_point;
                record.set_face_normal(ray, computeBodyNormal(hit_point));
                record.material = _material;
                record.u = std::atan2(hit_point.z(), hit_point.x()) * Math::INV_TWO_PI + 0.5;
                record.v = y / _height;

                closest_t = t;
                found_hit = true;
            }
        }
    }

    return found_hit;
}

bool LimitedCylinder::checkCapIntersection(const Ray& ray, double cap_y, const Vec3& normal,
                                     double t_min, double& closest_t, HitRecord& record) const {
    if (std::abs(ray.direction().y()) <= 1e-8) {
        return false;
    }

    double t = (cap_y - ray.origin().y()) / ray.direction().y();

    if (t <= t_min || t >= closest_t) {
        return false;
    }

    Vec3 hit_point = ray.at(t);

    if (!isWithinCapRadius(hit_point)) {
        return false;
    }

    record.t = t;
    record.point = hit_point;
    record.set_face_normal(ray, normal);
    record.material = _material;
    record.u = hit_point.x() / (2.0 * _radius) + 0.5;
    record.v = hit_point.z() / (2.0 * _radius) + 0.5;

    closest_t = t;
    return true;
}

bool LimitedCylinder::isWithinHeight(double y_coord) const {
    return y_coord >= 0 && y_coord <= _height;
}

bool LimitedCylinder::isWithinCapRadius(const Vec3& hit_point) const {
    double dx = hit_point.x();
    double dz = hit_point.z();
    return dx * dx + dz * dz <= _radius * _radius;
}

Vec3 LimitedCylinder::computeBodyNormal(const Vec3& hit_point) const {
    return normalize(Vec3(hit_point.x(), 0, hit_point.z()));
}

extern "C" IShape* create(double rx, double ry, double rz, double tx, double ty, double tz, double radius, double height, std::shared_ptr<IMaterial>* material) {
    return new LimitedCylinder(Vec3(rx, ry, rz), Vec3(tx, ty, tz), radius, height, *material);
}

extern "C" PluginMetadata* metadata() {
    static PluginMetadata metadata = {
        .pluginName = "limited_cylinder",
        .pluralForm = "limited_cylinders",
        .helpText = "LimitedCylinder (position (x, y, z), radius, height, material, [rotation (x, y, z)])",
        .category = "shape"
    };
    return &metadata;
}