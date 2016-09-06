#include "catch.hpp"
#include "tomo.hpp"

using T = float;

TEST_CASE("We can create reconstruction volumes", "[core]") {
    SECTION("2D") {
        int k = 16;
        tomo::volume<2_D> v(k, k);
        REQUIRE(v.cells() == k * k);
    }

    SECTION("3D") {
        int k = 16;
        tomo::volume<3_D> v(k, k, k);
        REQUIRE(v.cells() == k * k * k);
    }
}

TEST_CASE("We can initialize geometry", "[core]") {
    SECTION("2D") {
        int k = 16;
        auto v = tomo::volume<2_D>(k, k);
        auto g = tomo::parallel_geometry<2_D, T>(180, 250, v);
        CHECK(g.lines() == 180 * 250);

        int i = 0;
        for (auto line : g)
            ++i;
        CHECK(i == g.lines());
    }

    SECTION("3D") {
        int k = 16;
        auto v = tomo::volume<3_D>(k, k, k);
        auto g = tomo::parallel_geometry<3_D, T>(180, 250, v);
        CHECK(g.lines() == 180 * 250 * 250);

        int i = 0;
        for (auto line : g)
            ++i;
        CHECK(i == g.lines());
    }
}

TEST_CASE("Geometry lines are not empty") {
    SECTION("3D") {
        int k = 16;
        auto v = tomo::volume<3_D>(k, k, k);
        auto g = tomo::parallel_geometry<3_D, T>(k, k, v);
        auto proj = tomo::closest_projector<3_D, T>(v);

        bool a_line_is_empty = false;
        for (auto line : g) {
            int i = 0;
            for (auto elem : proj(line)) {
                ++i;
            }
            if (i == 0) {
                a_line_is_empty = true;
                break;
            }
        }

        CHECK(!a_line_is_empty);
    }

    SECTION("random geometry") {
        int k = 16;
        auto v = tomo::volume<3_D>(k);
        auto g = tomo::random_list_geometry<3_D, T>(1000, v);
        auto proj = tomo::linear_projector<3_D, T>(v);

        bool a_line_is_empty = false;
        for (auto line : g) {
            int i = 0;
            for (auto elem : proj(line)) {
                ++i;
            }
            if (i == 0) {
                a_line_is_empty = true;
                break;
            }
        }

        CHECK(!a_line_is_empty);
    }
}

TEST_CASE("We can use projectors", "[core]") {
    // FIXME add test case where we check that for each line in any geometry,
    // the origin lies inside the imaging volume
    //
}