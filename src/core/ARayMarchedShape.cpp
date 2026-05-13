#include "ARayMarchedShape.hpp"

ARayMarchedShape::ARayMarchedShape(Vec3 rotation, Vec3 translation, Vec3 scale, std::shared_ptr<IMaterial> material,
                                   int maxSteps, double epsilon, double maxDist)
    : AShape(rotation, translation, scale, material), _maxSteps(maxSteps), _epsilon(epsilon), _maxDist(maxDist) {}

Vec3 ARayMarchedShape::computeNormal(const Vec3& point) const {
    double dx = distanceEstimator(point + Vec3(_epsilon, 0, 0)) - distanceEstimator(point - Vec3(_epsilon, 0, 0));
    double dy = distanceEstimator(point + Vec3(0, _epsilon, 0)) - distanceEstimator(point - Vec3(0, _epsilon, 0));
    double dz = distanceEstimator(point + Vec3(0, 0, _epsilon)) - distanceEstimator(point - Vec3(0, 0, _epsilon));
    return normalize(Vec3(dx, dy, dz));
}

bool ARayMarchedShape::hitLocal(const Ray& ray, HitRecord& record) const {
    double t = _epsilon * 2.0;
    for (int i = 0; i < _maxSteps; ++i) {
        Vec3 point = ray.at(t);
        double dist = distanceEstimator(point);

        if (std::abs(dist) < _epsilon) {
            record.t = t;
            record.point = point;
            record.normal = computeNormal(point);
            record.material = _material;
            record.set_face_normal(ray, record.normal);
            return true;
        }

        t += dist;

        if (t >= _maxDist || t < 0.0) return false;
    }

    return false;
}