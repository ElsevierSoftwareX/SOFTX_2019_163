#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "python_common.hpp"
#include "tomo.hpp"

#ifdef USE_CUDA
#include "tomo_cuda.hpp"
#endif

using namespace tomo::python;

template <typename Sinogram>
std::array<int, 2> sino_dimensions(Sinogram& sino) {
    return std::array<int, 2>{sino.geometry().groups().x,
                              sino.geometry().groups().y};
}

void init_image(py::module& m) {
    py::class_<tomo::volume<2_D>>(m, "volume").def(py::init<int, int>());

    py::class_<tomo::image<2_D, double>>(m, "image")
        .def(py::init<tomo::volume<2_D>>())
        .def("data", &tomo::image<2_D, double>::mutable_data,
             "obtain the underlying image data")
        .def("dimensions", &tomo::image<2_D, double>::dimensions,
             "obtain the image dimensions");

    m.def("modified_sl_phantom", &tomo::modified_shepp_logan_phantom<double>,
          "create phantom");
}

template <typename Ps, typename Gs>
void init_geometry(py::module& m, Ps ps, Gs gs) {
    hana::for_each(gs, [&](auto x) {
        using G = typename decltype(+x[1_c])::type;
        auto name = x[0_c] + "_geometry"s;
        using Init = typename decltype(
            hana::unpack(x[2_c], hana::template_<py::detail::init>))::type;
        py::class_<G>(m, name.c_str()).def(Init());
    });

    auto combinations = hana::cartesian_product(hana::make_tuple(gs, ps));
    hana::for_each(combinations, [&](auto x) {
        using G = typename decltype(+(x[0_c][1_c]))::type;
        using P = typename decltype(+x[1_c][1_c])::type;
        using Sinogram = tomo::sinogram<2_D, T, G, P>;
        auto name = "sinogram_" + x[0_c][0_c] + "_"s + x[1_c][0_c];
        py::class_<Sinogram>(m, name.c_str())
            .def("data", &Sinogram::mutable_data,
                 "obtain the underlying image data")
            .def("dimensions", &sino_dimensions<Sinogram>,
                 "obtain the sinogram dimensions");
    });
}

template <typename Ps, typename Gs>
void init_operations(py::module& m, Ps ps, Gs gs) {
    hana::for_each(ps, [&](auto x) {
        auto name = x[0_c] + "_projector"s;
        using P = typename decltype(+x[1_c])::type;
        py::class_<P>(m, name.c_str()).def(py::init<tomo::volume<2_D>>());
    });

    auto combinations = hana::cartesian_product(hana::make_tuple(gs, ps));
    hana::for_each(combinations, [&](auto x) {
        using G = typename decltype(+(x[0_c][1_c]))::type;
        using P = typename decltype(+x[1_c][1_c])::type;
        m.def("forward_project", &tomo::forward_projection<2_D, T, G, P>);
    });
}

template <typename Ps, typename Gs>
void init_algorithm(py::module& m, Ps ps, Gs gs) {
    auto combinations = hana::cartesian_product(hana::make_tuple(gs, ps));
    hana::for_each(combinations, [&](auto x) {
        using G = typename decltype(+(x[0_c][1_c]))::type;
        using P = typename decltype(+x[1_c][1_c])::type;

        m.def("art", &tomo::art<2_D, T, G, P>, "ART reconstruction algorithm",
              py::arg("volume"), py::arg("geometry"), py::arg("projection"),
              py::arg("beta") = 0.5, py::arg("iterations") = 10);
        m.def("sart", &tomo::sart<2_D, T, G, P>,
              "SART reconstruction algorithm", py::arg("volume"),
              py::arg("geometry"), py::arg("projection"), py::arg("beta") = 0.5,
              py::arg("iterations") = 10);
        m.def("sirt", &tomo::sirt<2_D, T, G, P>,
              "SIRT reconstruction algorithm", py::arg("volume"),
              py::arg("geometry"), py::arg("projection"), py::arg("beta") = 0.5,
              py::arg("iterations") = 10);
    });
}

#ifdef USE_CUDA
template <typename Gs>
void init_cuda(py::module& m, Gs gs) {
    hana::for_each(gs, [&](auto x) {
        using G = typename decltype(+(x[1_c]))::type;
        m.def("cuda_forward_project",
              &tomo::cuda::forward_projection<2_D, T, G>);

        using Sinogram =
            tomo::sinogram<2_D, T, G, tomo::cuda::external_cuda_projector>;
        auto name = "sinogram_" + x[0_c] + "_cuda"s;
        py::class_<Sinogram>(m, name.c_str())
            .def("data", &Sinogram::mutable_data,
                 "obtain the underlying image data")
            .def("dimensions", &sino_dimensions<Sinogram>,
                 "obtain the sinogram dimensions");

        m.def("cuda_sart",
              &tomo::cuda::sart<2_D, T, G, tomo::cuda::external_cuda_projector>,
              "SART reconstruction algorithm using CUDA", py::arg("volume"),
              py::arg("geometry"), py::arg("projection"), py::arg("beta") = 0.5,
              py::arg("iterations") = 10);
    });
}
#endif

PYBIND11_PLUGIN(py_galactica) {
    py::module m("py_galactica", "bindings for galactica");

    // this is a list of the projector and geometry types, used to instantiate
    // the template algorithms and operations
    auto ps = hana::make_tuple(
        hana::make_tuple("linear"s,
                         hana::type_c<tomo::linear_projector<2_D, T>>),
        hana::make_tuple("joseph"s, hana::type_c<tomo::joseph_projector<T>>),
        hana::make_tuple("closest"s, hana::type_c<tomo::closest_projector<T>>));

    // the third entry is the signature of the constructor
    auto gs = hana::make_tuple(
        hana::make_tuple("parallel"s,
                         hana::type_c<tomo::parallel_geometry<2_D, T>>,
                         hana::tuple_t<int, int, tomo::volume<2_D>>),
        hana::make_tuple("list"s, hana::type_c<tomo::list_geometry<2_D, T>>,
                         hana::tuple_t<std::vector<tomo::line<2_D, double>>>));

    init_image(m);
    init_geometry(m, ps, gs);
    init_operations(m, ps, gs);
    init_algorithm(m, ps, gs);

#ifdef USE_CUDA
    init_cuda(m, gs);
#endif

    return m.ptr();
}
