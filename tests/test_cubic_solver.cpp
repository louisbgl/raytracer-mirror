#include <criterion/criterion.h>
#include "../src/Math/CubicSolver.hpp"
#include <cmath>

// Helper to check if two doubles are approximately equal
bool approx_equal_cubic(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

Test(cubic_solver, x3_minus_1) {
    // x^3 - 1 = 0  =>  roots: x = 1 (real), two complex roots
    CubicRoots result = CubicSolver::solve(-1, 0, 0, 1);

    cr_assert_eq(result.count, 1, "Expected 1 real root");
    cr_assert(approx_equal_cubic(result.roots[0], 1.0));
}

Test(cubic_solver, three_distinct_roots) {
    // (x-1)(x-2)(x-3) = x^3 - 6x^2 + 11x - 6 = 0
    CubicRoots result = CubicSolver::solve(-6, 11, -6, 1);

    cr_assert_eq(result.count, 3, "Expected 3 real roots");

    // Roots should be 1, 2, 3 in some order
    bool found[3] = {false, false, false};
    for (int i = 0; i < result.count; ++i) {
        if (approx_equal_cubic(result.roots[i], 1.0)) found[0] = true;
        if (approx_equal_cubic(result.roots[i], 2.0)) found[1] = true;
        if (approx_equal_cubic(result.roots[i], 3.0)) found[2] = true;
    }

    cr_assert(found[0] && found[1] && found[2],
              "All roots 1, 2, 3 should be found");
}

Test(cubic_solver, triple_root_at_zero) {
    // x^3 = 0  =>  root: x = 0 (triple)
    CubicRoots result = CubicSolver::solve(0, 0, 0, 1);

    cr_assert(result.count >= 1, "Expected at least 1 root");
    cr_assert(approx_equal_cubic(result.roots[0], 0.0), "Root should be 0");
}

Test(cubic_solver, double_root) {
    // (x-2)^2(x-3) = x^3 - 7x^2 + 16x - 12 = 0
    // Roots: x = 2 (double), x = 3
    CubicRoots result = CubicSolver::solve(-12, 16, -7, 1);

    cr_assert(result.count >= 2, "Expected at least 2 roots");

    // Should find 2 and 3 (2 might be reported twice or once depending on algorithm)
    bool found_2 = false;
    bool found_3 = false;
    for (int i = 0; i < result.count; ++i) {
        if (approx_equal_cubic(result.roots[i], 2.0)) found_2 = true;
        if (approx_equal_cubic(result.roots[i], 3.0)) found_3 = true;
    }

    cr_assert(found_2 && found_3, "Should find roots 2 and 3");
}
