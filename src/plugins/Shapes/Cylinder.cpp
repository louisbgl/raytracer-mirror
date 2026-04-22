#include "Cylinder.hpp"
#include "../../Math/QuadraticSolver.hpp"

Cylinder::Cylinder(Vec3 pos, double radius, std::shared_ptr<IMaterial> material)
    : _position(pos), _radius(radius), _material(material) {}

bool Cylinder::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    double closest_t = t_max;

    return checkBodyIntersection(ray, t_min, closest_t, record);
}

bool Cylinder::checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const {
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

            record.t = t;
            record.point = hit_point;
            record.set_face_normal(ray, computeBodyNormal(hit_point));
            record.material = _material;

            closest_t = t;
            found_hit = true;
        }
    }

    return found_hit;
}

Vec3 Cylinder::computeBodyNormal(const Vec3& hit_point) const {
    return normalize(Vec3(hit_point.x() - _position.x(), 0, hit_point.z() - _position.z()));
}

extern "C" IShape* create(double x, double y, double z, double radius, std::shared_ptr<IMaterial>* material) {
    return new Cylinder(Vec3(x, y, z), radius, *material);
}