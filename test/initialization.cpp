#include "catch.hpp"
#include "tomos/tomos.hpp"

using T = float;

TEST_CASE("We can create reconstruction volumes", "[core]") {
    SECTION("2D") {
        int k = 16;
        tomo::volume<2_D, T> v({k, k});
        REQUIRE(v.cells() == k * k);
    }

    SECTION("3D") {
        int k = 16;
        tomo::volume<3_D, T> v({k, k, k});
        REQUIRE(v.cells() == k * k * k);
    }

    SECTION("indexing") {
        int k = 16;
        tomo::volume<2_D, T> v2(k);
        tomo::volume<3_D, T> v3(k);

        CHECK(v2.index(2, 3) == 2 + 3 * k);
        CHECK(v2.unroll(v2.index(2, 3))[0] == 2);
        CHECK(v2.unroll(v2.index(2, 3))[1] == 3);

        CHECK(v3.index(2, 3, 5) == 2 + 3 * k + 5 * k * k);
        auto unrolled = v3.unroll(v3.index(2, 3, 5));
        CHECK(unrolled[0] == 2);
        CHECK(unrolled[1] == 3);
        CHECK(unrolled[2] == 5);
    }
}

/* 
TEST_CASE("We can initialize geometry", "[core]") {
    SECTION("2D") {
        int k = 16;
        auto v = tomo::volume<2_D, T>({k, k});
        auto g = tomo::geometry::parallel<2_D, T>(v, 180);
        auto lines = g.lines();
        CHECK(lines == 180 * k);

        int i = 0;
        for ([[maybe_unused]] auto line : g) {
            ++i;
        }
        CHECK(i == lines);
    }

    SECTION("3D") {
        int k = 16;
        auto v = tomo::volume<3_D, T>({k, k, k});
        auto g = tomo::geometry::parallel<3_D, T>(v, 180);
        CHECK(g.lines() == 180 * k * k);

        int i = 0;
        for ([[maybe_unused]] auto line : g) {
            ++i;
        }

        CHECK(i == g.lines());
    }
}

TEST_CASE("Geometry lines are not empty"){SECTION("3D"){int k = 16;
auto v = tomo::volume<3_D, T>({k, k, k});
auto g = tomo::geometry::parallel<3_D, T>(v, k);
auto proj = tomo::dim::closest<3_D, T>(v);

bool a_line_is_empty = false;
for (auto line : g) {
    int i = 0;
    for (auto elem : proj(line)) {
        (void)elem; // suppress unused warning
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
*/
