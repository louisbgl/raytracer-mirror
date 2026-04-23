#pragma once

#include <array>

/**
 * @file QuarticSolver.hpp
 * @brief C++ wrapper around Graphics Gems polynomial root solver
 *
 * ATTRIBUTION:
 * The underlying implementation uses unmodified code from Graphics Gems I:
 *   - Source: Graphics Gems I - "Roots3And4.c"
 *   - Author: Jochen Schwarze (schwarze@isa.de)
 *   - Date: January 26, 1990
 *   - Repository: https://github.com/erich666/GraphicsGems
 *   - License: Public Domain / Graphics Gems License (permissive)
 *
 * The original C code is preserved in src/Math/Roots3And4.c without modification.
 * This header provides only a modern C++ interface wrapper for type safety and
 * convenience. All credit for the solving algorithms goes to Jochen Schwarze.
 *
 * COEFFICIENT FORMAT:
 * For a polynomial: c[0] + c[1]*x + c[2]*x^2 + c[3]*x^3 + c[4]*x^4 = 0
 */

// External C functions from Graphics Gems (Roots3And4.c)
extern "C" {
    int SolveQuadric(double c[3], double s[2]);
    int SolveCubic(double c[4], double s[3]);
    int SolveQuartic(double c[5], double s[4]);
}

/**
 * @brief Result structure for quartic equation roots
 */
struct QuarticRoots {
    std::array<double, 4> roots;
    int count;

    bool hasRoots() const { return count > 0; }
};

/**
 * @brief Modern C++ wrapper for Graphics Gems polynomial root solver
 *
 * This class provides a clean, type-safe interface to the classic Graphics Gems
 * polynomial solver algorithms by Jochen Schwarze. The original C code is used
 * unmodified to preserve its proven numerical stability.
 */
class QuarticSolver {
public:
    /**
     * @brief Solve quartic equation: a + bx + cx^2 + dx^3 + ex^4 = 0
     * @param a Constant coefficient
     * @param b Linear coefficient
     * @param c Quadratic coefficient
     * @param d Cubic coefficient
     * @param e Quartic coefficient
     * @return QuarticRoots containing all real roots (count and values)
     *
     * Note: Uses the original Graphics Gems algorithm by Jochen Schwarze.
     * Complex roots are not returned (count will only include real roots).
     */
    static QuarticRoots solve(double a, double b, double c, double d, double e) {
        QuarticRoots result;
        result.count = 0;

        // Coefficients array for Graphics Gems format
        double coeffs[5] = {a, b, c, d, e};
        double roots[4];

        // Call the original Graphics Gems C function
        result.count = SolveQuartic(coeffs, roots);

        // Copy results to our C++ structure
        for (int i = 0; i < result.count; ++i) {
            result.roots[i] = roots[i];
        }

        return result;
    }
};
