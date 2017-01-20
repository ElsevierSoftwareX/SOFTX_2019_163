#pragma once

#include <limits>

#include "../common.hpp"
#include "../geometry.hpp"
#include "../math.hpp"
#include "../volume.hpp"

namespace tomo {
namespace geometry {

/**
 * Obtain the location of a detector depending on the dimension of the volume.
 *
 * \tparam T the scalar type to use
 *
 * \param detector the index of the detector
 * \param detector_count the total number of detectors
 * \param detector_step the distance between adjacent detectors
 *
 * \note the final parameter for the volume is used to choose the function for
 * the right dimension.
 */
template <typename T>
math::vec<1_D, T> detector_location(int detector, int detector_count,
                                    T detector_step, const volume<2_D>&) {
    return {(detector - (detector_count - 1) * (T)0.5) * detector_step};
}

/** ditto */
template <typename T>
math::vec2<T> detector_location(int detector, int detector_count,
                                T detector_step, const volume<3_D>&) {
    auto detector_x = detector % detector_count;
    auto detector_y = detector / detector_count;

    return {(detector_x - (detector_count - 1) * 0.5) * detector_step,
            (detector_y - (detector_count - 1) * 0.5) * detector_step};
}

/**
 * Obtain the line corresponding to a given location of a detector and a given
 * angle.
 *
 * \tparam T the scalar type to use
 *
 * \param current_detector the position of the detector
 * \param current_angle the angle of the view
 * \param vol the volume being scanned
 */
template <typename T>
inline math::ray<2_D, T> compute_line(math::vec<1_D, T> current_detector,
                                      T current_angle, const volume<2_D>& vol) {
    // some performance can be gained here by *not* shifting with image
    // center, and maybe we even want to cache these results somehow
    auto source = math::vec2<T>(-vol.x(), current_detector[0]);
    auto detector = math::vec2<T>(vol.x(), current_detector[0]);

    auto c = math::cos(-current_angle);
    auto s = math::sin(-current_angle);

    source = math::vec2<T>(c * source[0] - s * source[1],
                           s * source[0] + c * source[1]);
    detector = math::vec2<T>(c * detector[0] - s * detector[1],
                             s * detector[0] + c * detector[1]);

    auto image_center = math::vec2<T>(0.5 * vol.x(), 0.5 * vol.y());

    source += image_center;
    detector += image_center;

    return {source, detector};
}

/** ditto */
template <typename T>
inline math::ray<3_D, T> compute_line(math::vec2<T> current_detector,
                                      T current_angle, const volume<3_D>& vol) {
    // strategy: only consider current detector x, and ignore y, only add it at
    // the end
    auto volume_slice = volume<2_D>(vol.x(), vol.y());
    auto line_2d =
        compute_line({current_detector.x}, current_angle, volume_slice);

    auto source = math::vec3<T>(line_2d.source.x, line_2d.source.y,
                                current_detector.y + 0.5 * vol.z());
    auto detector = math::vec3<T>(line_2d.detector.x, line_2d.detector.y,
                                  current_detector.y + 0.5 * vol.z());

    return {source, detector};
}

/**
 * Geometry defined by parallel lines with a number of views.
 *
 * \tparam D the dimension of the volume.
 * \tparam T the scalar type to use
 */
template <dimension D, typename T>
class parallel : public base<D, T, parallel<D, T>> {
  public:
    using position = math::vec<D - 1, T>;

    /**
     * Construct the parallel geometry for a given number of angles and
     * detectors.
     *
     * \param angle_count the number of angles
     * \param detector_count the number of detectors
     * \param volume the volume being scanned
     */
    parallel(int angle_count, int detector_count, const volume<D>& volume)
        : base<D, T, parallel<D, T>>(angle_count *
                                     math::pow(detector_count, D - 1)),
          volume_(volume) {
        auto angle_step = math::pi<T> / angle_count;
        for (T angle = 0.0; angle < math::pi<T>; angle += angle_step) {
            angles_.push_back(angle);
        }

        int total_detector_count = math::pow(detector_count, D - 1);
        // FIXME this is only for equilateral volume
        auto detector_step = volume_.y() / (T)detector_count;
        for (int detector = 0; detector < total_detector_count; detector++) {
            detectors_.push_back(detector_location<T>(detector, detector_count,
                                                      detector_step, volume_));
        }

        this->dimensions_ = {detector_count, angle_count};
    }

    /** Obtain the number of detectors. */
    size_t detector_count() const { return detectors_.size(); }

    /** Obtain the number of angles. */
    size_t angle_count() const { return angles_.size(); }

    /** Obtain a vector containing each angle. */
    const std::vector<T>& angles() const { return angles_; }

    /** Obtain a vector containing the position of each detector. */
    const std::vector<position>& detectors() const { return detectors_; }

    /** Obtain a reference to the scanned volume. */
    const volume<D>& get_volume() const { return volume_; }

    /** Obtain the i-th line of the geometry. */
    inline math::ray<D, T> get_line(int i) const {
        return compute_line(detectors_[i % detector_count()],
                            angles_[i / detector_count()], volume_);
    }

  private:
    std::vector<T> angles_;
    std::vector<position> detectors_;
    volume<D> volume_;
};

} // namespace geometry
} // namespace tomo