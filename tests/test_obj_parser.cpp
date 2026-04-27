#include <criterion/criterion.h>
#include "../src/parsers/ObjParser.hpp"

Test(obj_parser, single_triangle) {
    ObjParser parser;
    auto shapes = parser.parse("tests/fixtures/triangle.obj", nullptr);

    cr_assert_eq(shapes.size(), 1, "Expected 1 triangle, got %zu", shapes.size());
}

Test(obj_parser, quad_splits_into_two_triangles) {
    ObjParser parser;
    auto shapes = parser.parse("tests/fixtures/quad.obj", nullptr);

    cr_assert_eq(shapes.size(), 2, "Expected quad to split into 2 triangles, got %zu", shapes.size());
}

Test(obj_parser, missing_file_throws) {
    ObjParser parser;
    cr_assert_throw(
        parser.parse("tests/fixtures/nonexistent.obj", nullptr),
        std::runtime_error
    );
}
