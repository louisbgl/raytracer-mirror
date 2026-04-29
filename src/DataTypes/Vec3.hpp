#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <cassert>

// C-style 3D vector struct to pass data across C interfaces
struct Vec3C {
    double x, y, z;
};

// Compile-time assert to ensure Vec3C is safe across C interfaces
static_assert(std::is_trivial_v<Vec3C>, "Vec3C must be a trivial type for C compatibility");
static_assert(std::is_standard_layout_v<Vec3C>, "Vec3C must have standard layout for C compatibility");

class Vec3 {
public:
    constexpr Vec3() noexcept : _x(0), _y(0), _z(0) {}
    constexpr Vec3(double x, double y, double z) noexcept : _x(x), _y(y), _z(z) {}
    constexpr Vec3(const Vec3C& c) noexcept : _x(c.x), _y(c.y), _z(c.z) {}

    [[nodiscard]] constexpr double x() const noexcept { return _x; }
    [[nodiscard]] constexpr double y() const noexcept { return _y; }
    [[nodiscard]] constexpr double z() const noexcept { return _z; }

    [[nodiscard]] constexpr Vec3 operator-() const noexcept { return Vec3(-_x, -_y, -_z); }

    [[nodiscard]] constexpr double operator[](size_t i) const noexcept {
        return (i == 0) ? _x : (i == 1) ? _y : _z;
    }

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        _x += other._x; _y += other._y; _z += other._z;
        return *this;
    }

    constexpr Vec3& operator*=(double scalar) noexcept {
        _x *= scalar; _y *= scalar; _z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(double scalar) noexcept {
        return *this *= (1.0 / scalar);
    }

    [[nodiscard]] constexpr Vec3C toCStruct() const noexcept {
        return Vec3C{_x, _y, _z};
    }

    [[nodiscard]] static constexpr Vec3 fromCStruct(const Vec3C& c) noexcept {
        return Vec3(c);
    }

private: 
    double _x, _y, _z;
};

[[nodiscard]] inline constexpr Vec3 operator+(const Vec3& left, const Vec3& right) noexcept {
    return Vec3(left.x() + right.x(), left.y() + right.y(), left.z() + right.z());
}

[[nodiscard]] inline constexpr Vec3 operator-(const Vec3& left, const Vec3& right) noexcept {
    return Vec3(left.x() - right.x(), left.y() - right.y(), left.z() - right.z());
}

[[nodiscard]] inline constexpr Vec3 operator*(double scalar, const Vec3& vec) noexcept {
    return Vec3(vec.x() * scalar, vec.y() * scalar, vec.z() * scalar);
}

[[nodiscard]] inline constexpr Vec3 operator*(const Vec3& vec, double scalar) noexcept {
    return scalar * vec;
}

[[nodiscard]] inline constexpr Vec3 operator*(const Vec3& left, const Vec3& right) noexcept {
    return Vec3(left.x() * right.x(), left.y() * right.y(), left.z() * right.z());
}

[[nodiscard]] inline constexpr Vec3 operator/(const Vec3& vec, double scalar) noexcept {
    return Vec3(vec.x() / scalar, vec.y() / scalar, vec.z() / scalar);
}

[[nodiscard]] inline constexpr double length_squared(const Vec3& vec) noexcept {
    return vec.x() * vec.x() + vec.y() * vec.y() + vec.z() * vec.z();
}

[[nodiscard]] inline double length(const Vec3& vec) noexcept {
    return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline constexpr double dot(const Vec3& left, const Vec3& right) noexcept {
    return left.x() * right.x() + left.y() * right.y() + left.z() * right.z();
}

[[nodiscard]] inline constexpr Vec3 cross(const Vec3& left, const Vec3& right) noexcept {
    return Vec3(
        left.y() * right.z() - left.z() * right.y(),
        left.z() * right.x() - left.x() * right.z(),
        left.x() * right.y() - left.y() * right.x()
    );
}

[[nodiscard]] inline Vec3 normalize(const Vec3& vec) noexcept {
    double len = length(vec);
    assert(len > 0.0 && "Cannot normalize zero vector");
    return vec / len;
}

inline std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
    os << "Vec3(" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
    return os;
}
