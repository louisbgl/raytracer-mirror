#pragma once

#include "AABB.hpp"
#include "../Interfaces/IShape.hpp"
#include <vector>
#include <memory>
#include <algorithm>

class BVHNode : public IShape {
public:
    BVHNode(std::vector<std::shared_ptr<IShape>>& shapes, size_t start, size_t end) {
        size_t count = end - start;

        if (count == 1) {
            _left = _right = shapes[start];
        } else if (count == 2) {
            _left  = shapes[start];
            _right = shapes[start + 1];
        } else {
            AABB centroidBox = shapes[start]->boundingBox();
            for (size_t i = start + 1; i < end; i++)
                centroidBox = AABB::surrounding_box(centroidBox, shapes[i]->boundingBox());

            Vec3 span = centroidBox.max() - centroidBox.min();
            int axis = 0;
            if (span.y() > span.x()) axis = 1;
            if (span.z() > span[axis]) axis = 2;

            std::sort(shapes.begin() + start, shapes.begin() + end,
                [axis](const std::shared_ptr<IShape>& a, const std::shared_ptr<IShape>& b) {
                    return a->boundingBox().min()[axis] < b->boundingBox().min()[axis];
                });

            size_t mid = start + count / 2;
            _left  = std::make_shared<BVHNode>(shapes, start, mid);
            _right = std::make_shared<BVHNode>(shapes, mid, end);
        }

        _box = AABB::surrounding_box(_left->boundingBox(), _right->boundingBox());
    }

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& record) const override {
        if (!_box.hit(ray, t_min, t_max))
            return false;

        bool hitLeft  = _left->hit(ray, t_min, t_max, record);
        bool hitRight = _right->hit(ray, t_min, hitLeft ? record.t : t_max, record);

        return hitLeft || hitRight;
    }

    AABB boundingBox() const override { return _box; }

private:
    std::shared_ptr<IShape> _left;
    std::shared_ptr<IShape> _right;
    AABB _box;
};
