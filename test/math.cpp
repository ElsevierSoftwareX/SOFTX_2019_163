#include "catch.hpp"
#include "tomo.hpp"

using T = float;

TEST_CASE("Basic operations on vectors", "[math]") {}

TEST_CASE("Intersection and box checking", "[math]") {
    SECTION("2D") {
        int k = 4;
        tomo::volume<2_D> v(k);
        std::vector<tomo::math::vec2<T>> xs = {
            {0, 0}, {k, k}, {k / 2, k / 2}, {k / 3, k / 2},
            {0, k}, {k, 0}, {1, 1}};
        std::vector<bool> is_inside = {false, false, true, true,
                                    false, false, true};
        assert(is_inside.size() == xs.size());

        bool can_compute_inside_correctly = true;
        for (std::size_t i = 0; i < is_inside.size(); ++i) {
            bool inside = tomo::math::inside<2_D, T>(xs[i], v);
            if(inside != is_inside[i]) {
                can_compute_inside_correctly = false;
                break;
            }
        }

        CHECK(can_compute_inside_correctly);
    }

    SECTION("3D") {
        int k = 4;
        tomo::volume<3_D> v(k);
        std::vector<tomo::math::vec3<T>> xs = {
            {0, 0, 0}, {k, k, 0}, {k / 2, k / 2, k / 2}, {k / 3, k / 2, k / 4},
            {0, k, 0}, {k, 0, 0}, {1, 1, 1}};
        std::vector<bool> is_inside = {false, false, true, true,
                                    false, false, true};
        assert(is_inside.size() == xs.size());

        bool can_compute_inside_correctly = true;
        for (std::size_t i = 0; i < is_inside.size(); ++i) {
            bool inside = tomo::math::inside<3_D, T>(xs[i], v);
            if(inside != is_inside[i]) {
                can_compute_inside_correctly = false;
                break;
            }
        }

        CHECK(can_compute_inside_correctly);
    }
}

TEST_CASE("Interpolation", "[math]") {
    SECTION("2D") {
        std::vector<tomo::math::matrix_element<T>> q;

        tomo::volume<2_D> v(8);
        tomo::math::vec<2_D, float> a(2.3);
        tomo::math::interpolate(a, v, q);

        CHECK(q.size() == 4);
    }

    SECTION("3D") {
        std::vector<tomo::math::matrix_element<T>> q;

        tomo::volume<3_D> v(8);
        tomo::math::vec<3_D, float> a(2.3);
        tomo::math::interpolate(a, v, q);

        CHECK(q.size() == 8);
        CHECK(q[0].index == 73);
    }
}