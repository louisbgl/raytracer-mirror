#include "LimitedCylinder.hpp"
#include "../../Math/QuadraticSolver.hpp"

LimitedCylinder::LimitedCylinder(Vec3 pos, double radius, double height, std::shared_ptr<IMaterial> material)
    : _position(pos), _radius(radius), _height(height), _material(material) {}

bool LimitedCylinder::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    double closest_t = t_max;
    bool hit_anything = false;

    hit_anything |= checkBodyIntersection(ray, t_min, closest_t, record);
    hit_anything |= checkCapIntersection(ray, _position.y(), Vec3(0, -1, 0), t_min, closest_t, record);
    hit_anything |= checkCapIntersection(ray, _position.y() + _height, Vec3(0, 1, 0), t_min, closest_t, record);

    return hit_anything;
}

bool LimitedCylinder::checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const {
    Vec3 oc = ray.origin() - _position;

    double a = ray.direction().x() * ray.direction().x() + ray.direction().z() * ray.direction().z();
    double b = 2.0 * (oc.x() * ray.direction().x() + oc.z() * ray.direction().z());
    double c = oc.x() * oc.x() + oc.z() * oc.z() - _radius * _radius;

    QuadraticRoots roots = QuadraticSolver::solve(a, b, c);
    if (!roots.hasRoots()) return false;

    bool found_hit = false;
    for (double t : {roots.t1, roots.t2}) {
        if (t > t_min && t < closest_t) {
            Vec3 hit_point = ray.at(t);
            double y = hit_point.y() - _position.y();

            if (isWithinHeight(y)) {
                record.t = t;
                record.point = hit_point;
                record.set_face_normal(ray, computeBodyNormal(hit_point));
                record.material = _material;

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

    closest_t = t;
    return true;
}

bool LimitedCylinder::isWithinHeight(double y_coord) const {
    return y_coord >= 0 && y_coord <= _height;
}

bool LimitedCylinder::isWithinCapRadius(const Vec3& hit_point) const {
    double dx = hit_point.x() - _position.x();
    double dz = hit_point.z() - _position.z();
    return dx * dx + dz * dz <= _radius * _radius;
}

Vec3 LimitedCylinder::computeBodyNormal(const Vec3& hit_point) const {
    return normalize(Vec3(hit_point.x() - _position.x(), 0, hit_point.z() - _position.z()));
}

extern "C" IShape* create(double x, double y, double z, double radius, double height, std::shared_ptr<IMaterial>* material) {
    return new LimitedCylinder(Vec3(x, y, z), radius, height, *material);
}