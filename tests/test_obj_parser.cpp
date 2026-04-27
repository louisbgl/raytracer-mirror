#include <criterion/criterion.h>
#include "../src/parsers/ObjParser.hpp"
#include "../src/DataTypes/Ray.hpp"
#include "../src/DataTypes/HitRecord.hpp"

// ── basic fixtures ────────────────────────────────────────────────────────────

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

// ── blender cube ──────────────────────────────────────────────────────────────

Test(obj_parser, blender_cube_triangle_count) {
    ObjParser parser;
    auto shapes = parser.parse("textures/obj/arthur/cube.obj", nullptr);
    // 6 quad faces → 12 triangles
    cr_assert_eq(shapes.size(), 12, "Expected 12 triangles, got %zu", shapes.size());
}

Test(obj_parser, blender_cube_top_face_normal) {
    ObjParser parser;
    auto shapes = parser.parse("textures/obj/arthur/cube.obj", nullptr);

    // Ray from above shooting straight down — hits the top face (y = 1)
    Ray ray(Vec3(0, 5, 0), Vec3(0, -1, 0));
    HitRecord record;
    bool hit = false;
    for (auto& shape : shapes) {
        if (shape->hit(ray, 0.001, 1000.0, record))
            hit = true;
    }

    cr_assert(hit, "Expected ray from above to hit the cube");
    cr_assert_float_eq(record.normal.y(), 1.0, 1e-4, "Expected top face normal to point up (0,1,0)");
    cr_assert_float_eq(record.normal.x(), 0.0, 1e-4);
    cr_assert_float_eq(record.normal.z(), 0.0, 1e-4);
}

Test(obj_parser, blender_cube_uv_in_range) {
    ObjParser parser;
    auto shapes = parser.parse("textures/obj/arthur/cube.obj", nullptr);

    Ray ray(Vec3(0, 5, 0), Vec3(0, -1, 0));
    HitRecord record;
    for (auto& shape : shapes) {
        if (shape->hit(ray, 0.001, 1000.0, record))
            break;
    }

    cr_assert(record.u >= 0.0 && record.u <= 1.0, "UV.u out of [0,1]: %f", record.u);
    cr_assert(record.v >= 0.0 && record.v <= 1.0, "UV.v out of [0,1]: %f", record.v);
}
