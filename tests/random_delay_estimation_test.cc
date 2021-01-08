#include <cstdlib>
#include <string>
#include <vector>

#include "catch2/catch.hpp"

#include "apm_data_dumper.h"
#include "echo_path_delay_estimator.h"
#include "render_delay_buffer.h"

#include "test_tools.h"

TEST_CASE("random sample should produce correct delay") {
  constexpr size_t kNumRenderChannels = 1;
  constexpr size_t kNumCaptureChannels = 1;
  constexpr int kSampleRateHz = 48000;
  constexpr size_t kNumBands = kSampleRateHz / 16000;

  constexpr size_t kDownSamplingFactors[] = {2, 4, 8};
  constexpr size_t kDelaySamples[] = {30, 64, 150, 200, 800, 4000};

  INFO("Creating render and capture buffers.");
  // Render buffer
  std::vector<std::vector<std::vector<float>>> render(
      kNumBands,
      std::vector<std::vector<float>>(kNumRenderChannels,
                                      std::vector<float>(webrtc::kBlockSize)));
  // Capture buffer
  std::vector<std::vector<float>> capture(
      kNumCaptureChannels, std::vector<float>(webrtc::kBlockSize));

  // Create no-op data dumper
  webrtc::ApmDataDumper data_dumper(0);
  for (auto down_sampling_factor : kDownSamplingFactors) {
    // Create echo detector configuration
    webrtc::EchoCanceller3Config config;
    config.delay.down_sampling_factor = down_sampling_factor;
    config.delay.num_filters = 10;

    INFO("Created a configuration object: {"
         << "down_sampling_factor: " << config.delay.down_sampling_factor
         << ", num_filters: " << config.delay.num_filters << "}");

    for (auto delay_samples : kDelaySamples) {
      SECTION("the down sampling factor is " +
              std::to_string(down_sampling_factor) +
              " and the delay sample count is " + std::to_string(delay_samples)) {

        std::unique_ptr<webrtc::RenderDelayBuffer> render_delay_buffer(
            webrtc::RenderDelayBuffer::Create(config, kSampleRateHz,
                                              kNumRenderChannels));
        DelayBuffer signal_delay_buffer(delay_samples);
        webrtc::EchoPathDelayEstimator estimator(&data_dumper, config,
                                                 kNumCaptureChannels);

        INFO("Start processing the blocks...");
        CAPTURE(down_sampling_factor, delay_samples,
                500 + delay_samples / webrtc::kBlockSize);

        absl::optional<webrtc::DelayEstimate> estimated_delay_samples;
        for (size_t i = 0; i < 500 + delay_samples / webrtc::kBlockSize; i++) {
          RandomizeSampleVector(render[0][0]);
          signal_delay_buffer.Delay(render[0][0], capture[0]);
          render_delay_buffer->Insert(render);

          if (i == 0) {
            render_delay_buffer->Reset();
          }

          render_delay_buffer->PrepareCaptureProcessing();

          auto estimate = estimator.EstimateDelay(
              render_delay_buffer->GetDownsampledRenderBuffer(), capture);

          if (estimate)
            estimated_delay_samples = estimate;
        }

        REQUIRE(estimated_delay_samples.has_value());

        // Allow estimated delay to be off by one sample in the
        // down-sampled domain.
        size_t delay_ds = delay_samples / down_sampling_factor;
        size_t estimated_delay_ds =
            estimated_delay_samples->delay / down_sampling_factor;
        REQUIRE(estimated_delay_ds >= delay_ds - 1);
        REQUIRE(estimated_delay_ds <= delay_ds + 1);
      }
    }
  }
}
