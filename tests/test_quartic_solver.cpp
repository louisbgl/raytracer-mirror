#include <criterion/criterion.h>
#include "../src/Math/QuarticSolver.hpp"
#include <cmath>

// Helper to check if two doubles are approximately equal
bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

Test(quartic_solver, x4_minus_1) {
    // x^4 - 1 = 0  =>  roots: x = ±1 (real), ±i (complex, ignored)
    QuarticRoots result = QuarticSolver::solve(-1, 0, 0, 0, 1);

    cr_assert_eq(result.count, 2, "Expected 2 real roots");
    cr_assert(approx_equal(std::abs(result.roots[0]), 1.0));
    cr_assert(approx_equal(std::abs(result.roots[1]), 1.0));
}

Test(quartic_solver, four_distinct_roots) {
    // (x-1)(x-2)(x-3)(x-4) = x^4 - 10x^3 + 35x^2 - 50x + 24 = 0
    QuarticRoots result = QuarticSolver::solve(24, -50, 35, -10, 1);

    cr_assert_eq(result.count, 4, "Expected 4 real roots");

    // Roots should be 1, 2, 3, 4 in some order
    bool found[4] = {false, false, false, false};
    for (int i = 0; i < result.count; ++i) {
        if (approx_equal(result.roots[i], 1.0)) found[0] = true;
        if (approx_equal(result.roots[i], 2.0)) found[1] = true;
        if (approx_equal(result.roots[i], 3.0)) found[2] = true;
        if (approx_equal(result.roots[i], 4.0)) found[3] = true;
    }

    cr_assert(found[0] && found[1] && found[2] && found[3],
              "All roots 1, 2, 3, 4 should be found");
}

Test(quartic_solver, no_real_roots) {
    // x^4 + 1 = 0  =>  no real roots (all complex)
    QuarticRoots result = QuarticSolver::solve(1, 0, 0, 0, 1);

    cr_assert_eq(result.count, 0, "Expected 0 real roots");
}

Test(quartic_solver, multiple_root_at_zero) {
    // x^4 = 0  =>  root: x = 0 (quadruple)
    QuarticRoots result = QuarticSolver::solve(0, 0, 0, 0, 1);

    cr_assert(result.count >= 1, "Expected at least 1 root");
    cr_assert(approx_equal(result.roots[0], 0.0), "Root should be 0");
}
