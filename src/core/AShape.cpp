#include "AShape.hpp"
#include <cmath>
#include <algorithm>

AShape::AShape(Vec3 rotation, Vec3 translation)
    : _rotation(rotation), _translation(translation), _aabbNeedsUpdate(true) {
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
    // INVERSE TRANSFORMATION (world → local)
    // Inverse:  world → un-translate → un-rotate → local

    Vec3 localOrigin = ray.origin() - _translation;

    double rx = _rotation.x() * M_PI / 180.0;
    double ry = _rotation.y() * M_PI / 180.0;
    double rz = _rotation.z() * M_PI / 180.0;

    Vec3 rotatedOrigin = applyInverseRotation(localOrigin, rx, ry, rz);
    Vec3 rotatedDirection = applyInverseRotation(ray.direction(), rx, ry, rz);

    return Ray(rotatedOrigin, rotatedDirection);
}

HitRecord AShape::localToWorld(const HitRecord& local, const Ray& worldRay) const {
    // FORWARD TRANSFORMATION (local → world)
    // Forward:  local → rotate → translate → world

    double rx = _rotation.x() * M_PI / 180.0;
    double ry = _rotation.y() * M_PI / 180.0;
    double rz = _rotation.z() * M_PI / 180.0;

    Vec3 worldPoint = applyRotation(local.point, rx, ry, rz);
    Vec3 worldNormal = applyRotation(local.normal, rx, ry, rz);

    worldPoint = worldPoint + _translation;

    worldNormal = normalize(worldNormal);

    HitRecord world;
    world.point = worldPoint;
    world.t = local.t;
    world.material = local.material;
    world.u = local.u;
    world.v = local.v;
    world.set_face_normal(worldRay, worldNormal);

    return world;
}

AABB AShape::transformAABB(const AABB& localAABB) const {
    // AABB TRANSFORMATION
    // 1. Transform all 8 corners of the box
    // 2. Find the new axis-aligned bounding box that contains all transformed corners

    Vec3 corners[8] = {
        Vec3(localAABB.min().x(), localAABB.min().y(), localAABB.min().z()),
        Vec3(localAABB.max().x(), localAABB.min().y(), localAABB.min().z()),
        Vec3(localAABB.min().x(), localAABB.max().y(), localAABB.min().z()),
        Vec3(localAABB.max().x(), localAABB.max().y(), localAABB.min().z()),
        Vec3(localAABB.min().x(), localAABB.min().y(), localAABB.max().z()),
        Vec3(localAABB.max().x(), localAABB.min().y(), localAABB.max().z()),
        Vec3(localAABB.min().x(), localAABB.max().y(), localAABB.max().z()),
        Vec3(localAABB.max().x(), localAABB.max().y(), localAABB.max().z())
    };

    double rx = _rotation.x() * M_PI / 180.0;
    double ry = _rotation.y() * M_PI / 180.0;
    double rz = _rotation.z() * M_PI / 180.0;

    Vec3 transformedCorner = applyRotation(corners[0], rx, ry, rz) + _translation;
    Vec3 minCorner = transformedCorner;
    Vec3 maxCorner = transformedCorner;

    for (int i = 1; i < 8; i++) {
        transformedCorner = applyRotation(corners[i], rx, ry, rz) + _translation;

        minCorner = Vec3(
            std::min(minCorner.x(), transformedCorner.x()),
            std::min(minCorner.y(), transformedCorner.y()),
            std::min(minCorner.z(), transformedCorner.z())
        );

        maxCorner = Vec3(
            std::max(maxCorner.x(), transformedCorner.x()),
            std::max(maxCorner.y(), transformedCorner.y()),
            std::max(maxCorner.z(), transformedCorner.z())
        );
    }

    return AABB(minCorner, maxCorner);
}

Vec3 AShape::applyRotation(const Vec3& v, double rx, double ry, double rz) const {
    // EULER ANGLE ROTATION
    // We use the ZYX rotation convention (intrinsic rotations):
    //   1. Rotate around Z-axis by rz
    //   2. Rotate around Y-axis by ry
    //   3. Rotate around X-axis by rx

    double cosX = std::cos(rx), sinX = std::sin(rx);
    double cosY = std::cos(ry), sinY = std::sin(ry);
    double cosZ = std::cos(rz), sinZ = std::sin(rz);

    // Apply the combined rotation matrix (Rx * Ry * Rz) to the vector
    double x = v.x() * (cosY * cosZ) +
               v.y() * (cosY * sinZ) +
               v.z() * (-sinY);

    double y = v.x() * (sinX * sinY * cosZ - cosX * sinZ) +
               v.y() * (sinX * sinY * sinZ + cosX * cosZ) +
               v.z() * (sinX * cosY);

    double z = v.x() * (cosX * sinY * cosZ + sinX * sinZ) +
               v.y() * (cosX * sinY * sinZ - sinX * cosZ) +
               v.z() * (cosX * cosY);

    return Vec3(x, y, z);
}

Vec3 AShape::applyInverseRotation(const Vec3& v, double rx, double ry, double rz) const {
    // INVERSE ROTATION = TRANSPOSE OF ROTATION MATRIX
    // The forward rotation matrix R = Rx * Ry * Rz has the form shown in applyRotation()
    // R^(-1) = R^T (transpose), which we compute by swapping rows/columns

    double cosX = std::cos(rx), sinX = std::sin(rx);
    double cosY = std::cos(ry), sinY = std::sin(ry);
    double cosZ = std::cos(rz), sinZ = std::sin(rz);

    // Transpose of the rotation matrix
    // Original matrix columns become rows in transpose
    double x = v.x() * (cosY * cosZ) +
               v.y() * (sinX * sinY * cosZ - cosX * sinZ) +
               v.z() * (cosX * sinY * cosZ + sinX * sinZ);

    double y = v.x() * (cosY * sinZ) +
               v.y() * (sinX * sinY * sinZ + cosX * cosZ) +
               v.z() * (cosX * sinY * sinZ - sinX * cosZ);

    double z = v.x() * (-sinY) +
               v.y() * (sinX * cosY) +
               v.z() * (cosX * cosY);

    return Vec3(x, y, z);
}
