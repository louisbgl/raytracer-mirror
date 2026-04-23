#include "Cylinder.hpp"
#include "../../Math/QuadraticSolver.hpp"

Cylinder::Cylinder(Vec3 pos, Vec3 axis, double radius, std::shared_ptr<IMaterial> material)
    : _position(pos), _axis(normalize(axis)), _radius(radius), _material(material) {}

bool Cylinder::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    double closest_t = t_max;

    return checkBodyIntersection(ray, t_min, closest_t, record);
}

bool Cylinder::checkBodyIntersection(const Ray& ray, double t_min, double& closest_t, HitRecord& record) const {
    Vec3 oc = ray.origin() - _position;

    Vec3 d_perp = ray.direction() - _axis * dot(ray.direction(), _axis);
    Vec3 oc_perp = oc - _axis * dot(oc, _axis);

    double a = dot(d_perp, d_perp);
    double b = 2.0 * dot(oc_perp, d_perp);
    double c = dot(oc_perp, oc_perp) - _radius * _radius;

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
    Vec3 p = hit_point - _position;
    Vec3 p_perp = p - _axis * dot(p, _axis);
    return normalize(p_perp);
}

extern "C" IShape* create(double x, double y, double z, double ax, double ay, double az, double radius, std::shared_ptr<IMaterial>* material) {
    return new Cylinder(Vec3(x, y, z), Vec3(ax, ay, az), radius, *material);
}