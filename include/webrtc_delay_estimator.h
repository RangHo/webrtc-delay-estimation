#ifndef DELAY_ESTIMATOR_H_
#define DELAY_ESTIMATOR_H_

#include <cstddef>
#include <exception>
#include <string>
#include <vector>

namespace webrtc_delay_estimation {

/**
 * Structure that holds information about WAV files.
 *
 * Note that the sample must be supplied in float, and int16 sampling is not
 * supported.
 */
struct WavFileInfo {

  /**
   * Sampling rate of the WAV file.
   */
  size_t sample_rate;

  /**
   * Number of channels present in the WAV file.
   */
  size_t num_channels;

  /**
   * Vector of all samples in the WAV file.
   */
  std::vector<float> samples;
};

/**
 * Structure that holds setting information.
 */
struct Setting {
  
  /**
   * Down sampling factor to use when estimating.
   */
  size_t down_sampling_factor;

  /**
   * Number of filters to apply when estimating.
   */
  size_t num_filters;
};

/**
 * Exception that represents there is no viable estimation result available.
 */
class NoEstimateAvailableError final : std::exception {
 public:
  const char* what() const noexcept override;
};

/**
 * Exception that represents the input files are not compatible.
 */
class IncompatibleInputsError final : std::exception {
 public:
  const char* what() const noexcept override;

};

size_t EstimateDelay(WavFileInfo& render,
                     WavFileInfo& capture,
                     Setting setting);

}  // namespace webrtc_delay_estimation

#endif
