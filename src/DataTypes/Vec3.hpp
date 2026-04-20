#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>

class Vec3 {
public:
    constexpr Vec3() : _x(0), _y(0), _z(0) {}
    constexpr Vec3(double x, double y, double z) : _x(x), _y(y), _z(z) {}

    constexpr double x() const { return _x; }
    constexpr double y() const { return _y; }
    constexpr double z() const { return _z; }

    constexpr Vec3 operator-() const { return Vec3(-_x, -_y, -_z); }

    constexpr double operator[](size_t i) const {
        return (i == 0) ? _x : (i == 1) ? _y : _z;
    }

    constexpr Vec3& operator+=(const Vec3& other) {
        _x += other._x; _y += other._y; _z += other._z;
        return *this;
    }

    constexpr Vec3& operator*=(double scalar) {
        _x *= scalar; _y *= scalar; _z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(double scalar) {
        return *this *= (1.0 / scalar);
    }

private: 
    double _x, _y, _z;
};

// --- Non-member Utility Functions ---

inline constexpr Vec3 operator+(const Vec3& left, const Vec3& right) {
    return Vec3(left.x() + right.x(), left.y() + right.y(), left.z() + right.z());
}

inline constexpr Vec3 operator-(const Vec3& left, const Vec3& right) {
    return Vec3(left.x() - right.x(), left.y() - right.y(), left.z() - right.z());
}

inline constexpr Vec3 operator*(double scalar, const Vec3& vec) {
    return Vec3(vec.x() * scalar, vec.y() * scalar, vec.z() * scalar);
}

inline constexpr Vec3 operator*(const Vec3& vec, double scalar) {
    return scalar * vec;
}

inline constexpr Vec3 operator*(const Vec3& left, const Vec3& right) {
    return Vec3(left.x() * right.x(), left.y() * right.y(), left.z() * right.z());
}

inline constexpr Vec3 operator/(const Vec3& vec, double scalar) {
    return Vec3(vec.x() / scalar, vec.y() / scalar, vec.z() / scalar);
}

inline constexpr double length_squared(const Vec3& vec) {
    return vec.x() * vec.x() + vec.y() * vec.y() + vec.z() * vec.z();
}

inline double length(const Vec3& vec) {
    return std::sqrt(length_squared(vec));
}

inline constexpr double dot(const Vec3& left, const Vec3& right) {
    return left.x() * right.x() + left.y() * right.y() + left.z() * right.z();
}

inline constexpr Vec3 cross(const Vec3& left, const Vec3& right) {
    return Vec3(
        left.y() * right.z() - left.z() * right.y(),
        left.z() * right.x() - left.x() * right.z(),
        left.x() * right.y() - left.y() * right.x()
    );
}

inline Vec3 normalize(const Vec3& vec) {
    double len = length(vec);
    return Vec3(vec.x() / len, vec.y() / len, vec.z() / len);
}

inline std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
    os << "Vec3(" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
    return os;
}
