#include "Matrix4x4.hpp"
#include "Constants.hpp"

Matrix4x4::Matrix4x4() {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            _m[i][j] = 0.0;
}

Matrix4x4 Matrix4x4::identity() {
    Matrix4x4 mat;
    for (int i = 0; i < 4; ++i)
        mat._m[i][i] = 1.0;
    return mat;
}

Matrix4x4 Matrix4x4::translate(const Vec3& t) {
    Matrix4x4 mat = Matrix4x4::identity();
    mat._m[0][3] = t.x();
    mat._m[1][3] = t.y();
    mat._m[2][3] = t.z();
    return mat;
}

Matrix4x4 Matrix4x4::scale(const Vec3& s) {
    Matrix4x4 mat;
    mat._m[0][0] = s.x();
    mat._m[1][1] = s.y();
    mat._m[2][2] = s.z();
    mat._m[3][3] = 1.0;
    return mat;
}

Matrix4x4 Matrix4x4::rotate(const Vec3& eulerDegrees) {
    Vec3 radians = eulerDegrees * (Math::PI / 180.0);
    double cx = std::cos(radians.x());
    double sx = std::sin(radians.x());
    double cy = std::cos(radians.y());
    double sy = std::sin(radians.y());
    double cz = std::cos(radians.z());
    double sz = std::sin(radians.z());

    Matrix4x4 rX = Matrix4x4::identity();
    rX._m[1][1] = cx; rX._m[1][2] = -sx;
    rX._m[2][1] = sx; rX._m[2][2] = cx;

    Matrix4x4 rY = Matrix4x4::identity();
    rY._m[0][0] = cy; rY._m[0][2] = sy;
    rY._m[2][0] = -sy; rY._m[2][2] = cy;

    Matrix4x4 rZ = Matrix4x4::identity();
    rZ._m[0][0] = cz; rZ._m[0][1] = -sz;
    rZ._m[1][0] = sz; rZ._m[1][1] = cz;

    return rX * rY * rZ;
}
 
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result._m[i][j] = 0.0;
            for (int k = 0; k < 4; ++k) {
                result._m[i][j] += _m[i][k] * other._m[k][j];
            }
        }
    }
    return result;
}

Vec3 Matrix4x4::transformPoint(const Vec3& p) const {
    double x = _m[0][0] * p.x() + _m[0][1] * p.y() + _m[0][2] * p.z() + _m[0][3];
    double y = _m[1][0] * p.x() + _m[1][1] * p.y() + _m[1][2] * p.z() + _m[1][3];
    double z = _m[2][0] * p.x() + _m[2][1] * p.y() + _m[2][2] * p.z() + _m[2][3];
    double w = _m[3][0] * p.x() + _m[3][1] * p.y() + _m[3][2] * p.z() + _m[3][3];
    if (w != 0.0) {
        x /= w; y /= w; z /= w;
    }
    return Vec3(x, y, z);
}

Vec3 Matrix4x4::transformDirection(const Vec3& d) const {
    double x = _m[0][0] * d.x() + _m[0][1] * d.y() + _m[0][2] * d.z();
    double y = _m[1][0] * d.x() + _m[1][1] * d.y() + _m[1][2] * d.z();
    double z = _m[2][0] * d.x() + _m[2][1] * d.y() + _m[2][2] * d.z();
    return Vec3(x, y, z);
}

Matrix4x4 Matrix4x4::transposed() const {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result._m[i][j] = _m[j][i];
    return result;
}

Matrix4x4 Matrix4x4::inverse() const {
    double s0 = _m[0][0]*_m[1][1] - _m[0][1]*_m[1][0];
    double s1 = _m[0][0]*_m[1][2] - _m[0][2]*_m[1][0];
    double s2 = _m[0][0]*_m[1][3] - _m[0][3]*_m[1][0];
    double s3 = _m[0][1]*_m[1][2] - _m[0][2]*_m[1][1];
    double s4 = _m[0][1]*_m[1][3] - _m[0][3]*_m[1][1];
    double s5 = _m[0][2]*_m[1][3] - _m[0][3]*_m[1][2];

    double c0 = _m[2][0]*_m[3][1] - _m[2][1]*_m[3][0];
    double c1 = _m[2][0]*_m[3][2] - _m[2][2]*_m[3][0];
    double c2 = _m[2][0]*_m[3][3] - _m[2][3]*_m[3][0];
    double c3 = _m[2][1]*_m[3][2] - _m[2][2]*_m[3][1];
    double c4 = _m[2][1]*_m[3][3] - _m[2][3]*_m[3][1];
    double c5 = _m[2][2]*_m[3][3] - _m[2][3]*_m[3][2];

    double det = s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0;
    if (std::abs(det) < 1e-10)
        throw std::runtime_error("Matrix4x4: singular matrix, cannot invert");

    double inv = 1.0 / det;

    Matrix4x4 result;
    result._m[0][0] = ( _m[1][1]*c5 - _m[1][2]*c4 + _m[1][3]*c3) * inv;
    result._m[0][1] = (-_m[0][1]*c5 + _m[0][2]*c4 - _m[0][3]*c3) * inv;
    result._m[0][2] = ( _m[3][1]*s5 - _m[3][2]*s4 + _m[3][3]*s3) * inv;
    result._m[0][3] = (-_m[2][1]*s5 + _m[2][2]*s4 - _m[2][3]*s3) * inv;

    result._m[1][0] = (-_m[1][0]*c5 + _m[1][2]*c2 - _m[1][3]*c1) * inv;
    result._m[1][1] = ( _m[0][0]*c5 - _m[0][2]*c2 + _m[0][3]*c1) * inv;
    result._m[1][2] = (-_m[3][0]*s5 + _m[3][2]*s2 - _m[3][3]*s1) * inv;
    result._m[1][3] = ( _m[2][0]*s5 - _m[2][2]*s2 + _m[2][3]*s1) * inv;

    result._m[2][0] = ( _m[1][0]*c4 - _m[1][1]*c2 + _m[1][3]*c0) * inv;
    result._m[2][1] = (-_m[0][0]*c4 + _m[0][1]*c2 - _m[0][3]*c0) * inv;
    result._m[2][2] = ( _m[3][0]*s4 - _m[3][1]*s2 + _m[3][3]*s0) * inv;
    result._m[2][3] = (-_m[2][0]*s4 + _m[2][1]*s2 - _m[2][3]*s0) * inv;

    result._m[3][0] = (-_m[1][0]*c3 + _m[1][1]*c1 - _m[1][2]*c0) * inv;
    result._m[3][1] = ( _m[0][0]*c3 - _m[0][1]*c1 + _m[0][2]*c0) * inv;
    result._m[3][2] = (-_m[3][0]*s3 + _m[3][1]*s1 - _m[3][2]*s0) * inv;
    result._m[3][3] = ( _m[2][0]*s3 - _m[2][1]*s1 + _m[2][2]*s0) * inv;

    return result;
}