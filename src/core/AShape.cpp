#include "AShape.hpp"

#include <algorithm>
#include <cmath>

AShape::AShape(Vec3 rotation, Vec3 translation, Vec3 scale) {
    _transform =
        Matrix4x4::translate(translation) * Matrix4x4::rotate(rotation) * Matrix4x4::scale(scale);
    _inverseTransform = _transform.inverse();
    _normalTransform = _inverseTransform.transposed();
    _aabbNeedsUpdate = true;
}

bool AShape::hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const {
    // Lazy AABB update on first hit call
    if (_aabbNeedsUpdate) {
        const_cast<AShape*>(this)->updateWorldAABB();
        const_cast<AShape*>(this)->_aabbNeedsUpdate = false;
    }

    // 1. Check world AABB first for early rejection
    if (!_worldAABB.hit(ray, t_min, t_max)) return false;

    // 2. Transform ray to local space
    Ray localRay = worldToLocal(ray);

    // 3. Perform local hit test
    HitRecord localRecord;
    if (!hitLocal(localRay, localRecord)) return false;

    // 4. Transform hit record back to world space
    record = localToWorld(localRecord, ray);

    // Recompute t in world space to keep ordering consistent after scaling.
    record.t = dot(record.point - ray.origin(), ray.direction());

    // 5. Validate t range in world space
    if (record.t < t_min || record.t > t_max) return false;
    return true;
}

AABB AShape::boundingBox() const {
    if (_aabbNeedsUpdate) {
        const_cast<AShape*>(this)->updateWorldAABB();
        const_cast<AShape*>(this)->_aabbNeedsUpdate = false;
    }
    return _worldAABB;
}

void AShape::updateWorldAABB() {
    AABB localAABB = computeLocalAABB();
    _worldAABB = transformAABB(localAABB);
}

Ray AShape::worldToLocal(const Ray& ray) const {
    Vec3 localOrigin = _inverseTransform.transformPoint(ray.origin());
    Vec3 localDirection = _inverseTransform.transformDirection(ray.direction());
    return Ray(localOrigin, localDirection);
}

HitRecord AShape::localToWorld(const HitRecord& local, const Ray& worldRay) const {
    Vec3 worldPoint = _transform.transformPoint(local.point);
    Vec3 worldNormal = _normalTransform.transformDirection(local.normal);

    worldNormal = normalize(worldNormal);
    HitRecord world(worldPoint, worldNormal, local.t, local.front_face, local.material);

    world.u = local.u;
    world.v = local.v;

    world.set_face_normal(worldRay, worldNormal);

    return world;
}

AABB AShape::transformAABB(const AABB& localAABB) const {
    Vec3 corners[8] = {Vec3(localAABB.min().x(), localAABB.min().y(), localAABB.min().z()),
                       Vec3(localAABB.max().x(), localAABB.min().y(), localAABB.min().z()),
                       Vec3(localAABB.min().x(), localAABB.max().y(), localAABB.min().z()),
                       Vec3(localAABB.max().x(), localAABB.max().y(), localAABB.min().z()),
                       Vec3(localAABB.min().x(), localAABB.min().y(), localAABB.max().z()),
                       Vec3(localAABB.max().x(), localAABB.min().y(), localAABB.max().z()),
                       Vec3(localAABB.min().x(), localAABB.max().y(), localAABB.max().z()),
                       Vec3(localAABB.max().x(), localAABB.max().y(), localAABB.max().z())};

    Vec3 first = _transform.transformPoint(corners[0]);
    Vec3 minCorner = first;
    Vec3 maxCorner = first;

    for (int i = 1; i < 8; i++) {
        Vec3 t = _transform.transformPoint(corners[i]);
        minCorner = Vec3(std::min(minCorner.x(), t.x()), std::min(minCorner.y(), t.y()),
                         std::min(minCorner.z(), t.z()));
        maxCorner = Vec3(std::max(maxCorner.x(), t.x()), std::max(maxCorner.y(), t.y()),
                         std::max(maxCorner.z(), t.z()));
    }
    return AABB(minCorner, maxCorner);
}
