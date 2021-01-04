#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "render_delay_buffer.h"
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

#include "apm_data_dumper.h"
#include "echo_path_delay_estimator.h"
#include "utilities.h"
#include "wav_file.h"

// Compile time constants
// TODO: they might have to be exposed as command line arguments
static constexpr size_t band_size = 1;
static constexpr size_t block_size = 64;
static constexpr size_t down_sampling_factor = 8;

int main(int argc, char* argv[]) {
  // Setup logger
  spdlog::cfg::load_env_levels();
  spdlog::set_pattern("[%l] %v");

  // Insane command line arguments
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <render> <capture>" << std::endl;
    std::exit(1);
  }

  // Make sure that the files exist first
  if (!exists(argv[1])) {
    std::cerr << argv[1] << ": No such file or directory" << std::endl;
    std::exit(1);
  }
  if (!exists(argv[2])) {
    std::cerr << argv[2] << ": No such file or directory" << std::endl;
    std::exit(1);
  }

  // Construct WAV reader from filenames provided
  webrtc::WavReader rendered(argv[1]);
  webrtc::WavReader captured(argv[2]);

  // Some debug infomration about each input files
  spdlog::debug("Render file information:");
  spdlog::debug("  sample rate: {}", rendered.sample_rate());
  spdlog::debug("  number of channels: {}", rendered.num_channels());
  spdlog::debug("  number of samples: {}", rendered.num_samples());
  spdlog::debug("Capture file information:");
  spdlog::debug("  sample rate: {}", captured.sample_rate());
  spdlog::debug("  number of channels: {}", captured.num_channels());
  spdlog::debug("  number of samples: {}", captured.num_samples());

  // If the files have different sampling rate or different amount of channels,
  if (rendered.sample_rate() != captured.sample_rate() ||
      rendered.num_channels() != captured.num_channels()) {
    spdlog::error("Render and capture files are incompatible. Cannot proceed.");
    spdlog::error("  Render file: {} {}, {} Hz", rendered.num_channels(),
                  rendered.num_channels() == 1 ? "channel" : "channels",
                  rendered.sample_rate());
    spdlog::error("  Render file: {} {}, {} Hz", captured.num_channels(),
                  captured.num_channels() == 1 ? "channel" : "channels",
                  captured.sample_rate());
    std::exit(2);
  }

  // Extract information from the file metadata
  auto sample_rate = rendered.sample_rate();
  auto num_channels = rendered.num_channels();
  auto num_samples = rendered.num_samples() < captured.num_samples()
                         ? rendered.num_samples()
                         : captured.num_samples();

  // Setup structures and classes to use EchoPathDeayEstimator
  webrtc::ApmDataDumper data_dumper(0);  // basically NOP
  webrtc::EchoCanceller3Config config;
  config.delay.down_sampling_factor =
      2;                          // downsampling factor used to detect delay
  config.delay.num_filters = 10;  // TODO: no idea what this does

  // render_buffer[band][channel][block]
  std::vector<std::vector<std::vector<float>>> render_buffer(
      band_size, std::vector<std::vector<float>>(
                     rendered.num_channels(), std::vector<float>(block_size)));
  // capture_buffer[channel][block]
  std::vector<std::vector<float>> capture_buffer(
      captured.num_channels(), std::vector<float>(block_size));

  // Render delay buffer required to create downsampled render buffer
  std::unique_ptr<webrtc::RenderDelayBuffer> render_delay_buffer(
      webrtc::RenderDelayBuffer::Create(config, sample_rate, num_channels));

  // Actual estimator object
  webrtc::EchoPathDelayEstimator estimator(&data_dumper, config,
                                           captured.num_channels());

  // Loop through the entire sample to find the best delay value
  absl::optional<webrtc::DelayEstimate> estimated_delay;
  for (int i = 0; i < num_samples / block_size; i++) {
    rendered.ReadSamples(block_size, render_buffer[0][0].data());
    captured.ReadSamples(block_size, capture_buffer[0].data());

    render_delay_buffer->Insert(render_buffer);
    render_delay_buffer->PrepareCaptureProcessing();

    auto maybe_estimated_delay = estimator.EstimateDelay(
        render_delay_buffer->GetDownsampledRenderBuffer(), capture_buffer);

    // Sometimes, there is a new updated value, sometimes, there isn't
    if (maybe_estimated_delay) {
      if (!estimated_delay) {
        spdlog::debug(
            "First delay estimation appeared at trial #{}: {} sample(s)", i,
            maybe_estimated_delay->delay);
      } else if (estimated_delay->delay != maybe_estimated_delay->delay) {
        spdlog::debug("Delay estimation updated at trial #{}: {} sample(s)", i,
                      maybe_estimated_delay->delay);
      }

      estimated_delay = maybe_estimated_delay;
    }
  }

  if (estimated_delay)
    spdlog::info("Estimated delay: {} samples.", estimated_delay->delay);
  else
    spdlog::critical("Delay estimate is unavailable.");

  return 0;
}
