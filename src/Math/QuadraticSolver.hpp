#pragma once

#include <cmath>

/**
 * @brief A structure to hold the roots of a quadratic equation and the number of roots found.
 * @note 0, 1 or 2 roots.
 */
struct QuadraticRoots {
    double t1 = 0.0;
    double t2 = 0.0;
    int count = 0;

    bool hasRoots() const { return count > 0; }
    bool isTangent() const { return count == 1; }
    
    double operator[](int i) const {
        return (i == 0) ? t1 : t2;
    }
};

/**
 * @brief A utility class to solve quadratic equations of the form ax^2 + bx + c = 0.
 * @note Use the quadratic formula.
 */
class QuadraticSolver {
public:
    static inline QuadraticRoots solve(double a, double b, double c) {
        QuadraticRoots roots;
        double discriminant = b * b - 4 * a * c;

        if (discriminant > EPSILON) {
            roots.t1 = (-b - std::sqrt(discriminant)) / (2 * a);
            roots.t2 = (-b + std::sqrt(discriminant)) / (2 * a);
            roots.count = 2;
        } else if (std::abs(discriminant) <= EPSILON) {
            roots.t1 = -b / (2 * a);
            roots.t2 = roots.t1;
            roots.count = 1;
        } else {
            roots.count = 0;
        }

        return roots;
    }
    
private:
    static constexpr double EPSILON = 1e-8;
};