#include "QuadraticSolver.hpp"

QuadraticRoots QuadraticSolver::solve(double a, double b, double c) {
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