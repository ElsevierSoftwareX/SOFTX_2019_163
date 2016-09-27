#pragma once

#include <array>
#include <numeric>

#include "common.hpp"
#include "math/vector.hpp"

namespace tomo {

/**
 * The region which is being imaged.
 *
 * The volume describes the resolution of the reconstructed image. This is used
 * to properly construct the geometry of the experiment, but it is also used in
 * the descritization and reconstruction methods themselves.
 *
 * \tparam D the dimension of the volume (and thus the reconstruction problem).
*/
template <dimension D>
class volume {
  public:
    /// Construct a cubic volume spanning `k` voxels on each axis.
    volume(int k) {
        for (int i = 0; i < D; ++i)
            dimensions_[i] = k;
    }

    /**
     * Construct a (hyper)rectangular volume.
     *
     * \param dimensions an array of which the i-th element corresponds to
     * the spanning width of the volume on the i-th axis.
     */
    volume(std::array<int, D> dimensions) : dimensions_(dimensions) {}

    /**
     * Construct a (hyper)rectangular volume.
     *
     * The i-th parameter corresponds to the spanning width of the volume on
     * the i-th axis.
     */
    template <typename... Ts>
    volume(Ts... dims) : dimensions_(std::array<int, D>{dims...}) {}

    /**
     * Obtain the size of the first dimension.
     *
     * \returns number of voxels in first dimension
     */
    int x() const { return dimensions_[0]; }

    /**
     * Obtain the size of the second dimension.
     *
     * \returns number of voxels in second dimension
     * \note only valid if D > 1
     */
    int y() const {
        static_assert(D > 1, "requesting 'y' in volume of dimension < 2");
        return dimensions_[1];
    }

    /**
     * Obtain the size of the third dimension.
     *
     * \returns number of voxels in third dimension
     * \note only valid if D > 2
     */
    int z() const {
        static_assert(D > 2, "requesting 'z' in volume of dimension < 3");
        return dimensions_[2];
    }

    /**
     * Obtain the size of a dimension
     *
     * \param i the index of the dimension
     * \returns number of voxels in the the i-th dimension
     */
    int operator[](size_t i) const { return dimensions_[i]; }

    /**
     * Obtain the index corresponding to a voxel
     *
     * \param xs a vector describing the voxel
     * \returns the index of the voxel
     */
    template <typename Vector>
    int index_by_vector(Vector xs) const {
        return index_by_vector_(xs);
    }

    /**
     * Obtain the index corresponding to a voxel
     *
     * \param xs an array describing the voxel
     * \returns the index of the voxel
     */
    int index(std::array<int, D> xs) const { return index_by_vector_(xs); }

    /**
     * Obtain the index corresponding to a voxel
     *
     * The i-th parameter corresponds to the index of the voxel in the i-th
     * dimension.
     *
     * \returns the index of the voxel
     */
    template <typename... Ts>
    int index(Ts... xs) const {
        return index_(0, 1, xs...);
    }

    /**
     * Obtain the dimensions of the volume.
     *
     * \returns an array whose i-th element corresponds to the spanning width
     * of the volume in the i-th axis.
     */
    std::array<int, D> dimensions() const { return dimensions_; }

    /**
     * Obtain the lengths of the sides of the volume.
     *
     * \returns a vector whose i-th element corresponds to the spanning width
     * of the volume in the i-th axis.
     */
    math::vec<D, int> lengths() const {
        math::vec<D, int> result;
        for (int i = 0; i < D; ++i) {
            result[i] = dimensions_[i];
        }
        return result;
    }

    /**
     * Obtain the total number of cells (voxels) in the volume.
     *
     * \returns number of voxels in the volume.
     */
    int cells() const {
        return std::accumulate(dimensions_.begin(), dimensions_.end(), 1,
                               std::multiplies<int>());
    }

  private:
    template <typename T, typename... Ts>
    int index_(int current, int offset, T x, Ts... xs) const {
        offset *= dimensions_[D - sizeof...(xs)];
        current += offset * x;
        return index_(current, offset, xs...);
    }

    template <typename Vector>
    inline int index_by_vector_(Vector xs) const {
        int result = xs[0];
        int offset = dimensions_[0];
        for (int i = 1; i < D; ++i) {
            result += offset * xs[i];
            offset *= dimensions_[i];
        }
        return result;
    }

    int index_(int current, int offset) const { return current; }

    std::array<int, D> dimensions_;
};

} // namespace tomo
