#pragma once

#include "../DataTypes/Vec3.hpp"
#include <cmath>
#include <stdexcept>

class Matrix4x4 {
public:
    Matrix4x4();

    // Static factories
    static Matrix4x4 identity();
    static Matrix4x4 translate(const Vec3& t);
    static Matrix4x4 rotate(const Vec3& eulerDegrees);
    static Matrix4x4 scale(const Vec3& s);

    // Composition
    Matrix4x4 operator*(const Matrix4x4& other) const;

    Vec3 transformPoint(const Vec3& p) const;     // w=1: translation applies
    Vec3 transformDirection(const Vec3& d) const; // w=0: translation ignored

    Matrix4x4 transposed() const;
    Matrix4x4 inverse() const;

private:
    double _m[4][4];
};
