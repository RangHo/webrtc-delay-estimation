#include <algorithm>
#include <cstddef>
#include <vector>

#include "catch2/catch.hpp"

#include "webrtc_delay_estimation.h"

#include "test_tools.h"

TEST_CASE("sample vector filled with random sample should produce correct delay", "[delay_estimation]") {
  using namespace webrtc_delay_estimation;

  constexpr size_t kNumRenderChannels = 1;
  constexpr size_t kNumCaptureChannels = 1;
  constexpr int kSampleRateHz = 48000;
  constexpr size_t kSampleSize = 15000;

  constexpr size_t kDownSamplingFactors[] = {2, 4, 8};
  constexpr size_t kDelaySamples[] = {30, 64, 150, 200, 800, 4000};

  std::vector<float> render(kSampleSize);
  RandomizeSampleVector(render);
  INFO("Filled the render buffer with " << render.size() << " samples.");

  for (auto ds_factor : kDownSamplingFactors) {
    Setting setting;
    setting.down_sampling_factor = ds_factor;
    setting.num_filters = 10;

    for (auto delay : kDelaySamples) {
      SECTION("the down sampling factor is " +
              std::to_string(ds_factor) +
              " and the delay sample count is " + std::to_string(delay)) {
        
        // Create capture vector and copy the render buffer with delay
        std::vector<float> capture(kSampleSize + delay);
        std::fill(capture.begin(), std::next(capture.begin(), delay), 0.0f);
        std::copy(render.begin(), render.end(), std::next(capture.begin(), delay));

        // Construct WAV info structure
        WavFileInfo render_info;
        render_info.num_channels = kNumRenderChannels;
        render_info.sample_rate = kSampleRateHz;
        render_info.samples = render;
        WavFileInfo capture_info;
        capture_info.num_channels = kNumCaptureChannels;
        capture_info.sample_rate = kSampleRateHz;
        capture_info.samples = capture;
        
        // Call delay estimator
        size_t result;
        try {
          result = EstimateDelay(render_info, capture_info, setting);
        } catch (std::exception e) {
          FAIL("Unable to get estimated delay value: " << e.what());
        }

        // Allow estimated delay to be off by one sample in the down-sampled domain.
        size_t delay_ds = delay / ds_factor;
        size_t estimated_delay_ds = result / ds_factor;
        REQUIRE(estimated_delay_ds >= delay_ds - 1);
        REQUIRE(estimated_delay_ds <= delay_ds + 1);
      }
    }
  }
}

