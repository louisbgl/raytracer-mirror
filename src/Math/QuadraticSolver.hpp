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
    static QuadraticRoots solve(double a, double b, double c);
    
private:
    static constexpr double EPSILON = 1e-8;
};