#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "cxxopts.hpp"

#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

#include "apm_data_dumper.h"
#include "echo_path_delay_estimator.h"
#include "render_delay_buffer.h"
#include "wav_file.h"

// Compile time constants
static const constexpr char* default_down_sampling_factor = "8";

static bool exists(const std::string& filename) {
  std::ifstream file_to_test(filename);
  return file_to_test.good();
}

int main(int argc, char* argv[]) {
  // Setup logger
  spdlog::set_pattern("[%l] %v");

  // Path to WAV input files
  std::string render_filename, capture_filename;

  // Arguments used when recognizing delay
  size_t down_sampling_factor;

  // Whether the output should be brief
  bool brief_output = false;

  // Parse command line arguments
  try {
    // clang-format butchers readability when defining options so it is better
    // to just ignore the 80-column limit
    // clang-format off
    cxxopts::Options options(argv[0], "Delay estimation algorithm extracted from WebRTC library.");
    options.add_options()
      ("h,help", "Display help message.")
      ("v,verbose", "Makes this program talk more.")
      ("brief", "Makes this program talk only when absolutely necessary.",
          cxxopts::value(brief_output))
      ("d,downsampling-factor", "Down-sampling factor to use when recognizing delay.",
          cxxopts::value(down_sampling_factor)->default_value(default_down_sampling_factor))
      ("render", "Path to the \"rendered\" WAV file.",
          cxxopts::value(render_filename))
      ("capture", "Path to the \"captured\" WAV file.",
          cxxopts::value(capture_filename));
    options.positional_help("<render> <capture>");
    // clang-format on
    options.parse_positional({"render", "capture", "positional"});
    auto args = options.parse(argc, argv);

    // Display help message first
    if (args.count("help")) {
      std::cerr << options.help() << std::endl;
      std::exit(0);
    }

    // Both positional arguments are required
    if (!args.count("render") || !args.count("capture")) {
      spdlog::error("USAGE: {} <render> <capture>", argv[0]);
      spdlog::error("Try '{} --help' for more information.", argv[0]);
      std::exit(2);
    }

    // Change log level according to the verbosity
    if (args.count("verbose"))
      spdlog::set_level(spdlog::level::debug);

    // If brief, disable logging unless some error happens
    if (args.count("brief"))
      spdlog::set_level(spdlog::level::err);

  } catch (const cxxopts::OptionException& e) {
    spdlog::error("Unable to parse options: {}", e.what());
    std::exit(255);
  }

  // Make sure that the files exist first
  if (!exists(render_filename)) {
    std::cerr << render_filename << ": No such file or directory" << std::endl;
    std::exit(1);
  }
  if (!exists(capture_filename)) {
    std::cerr << capture_filename << ": No such file or directory" << std::endl;
    std::exit(1);
  }

  spdlog::debug("---- Reading input files ----");

  // Construct WAV reader from filenames provided
  webrtc::WavReader rendered(render_filename);
  webrtc::WavReader captured(capture_filename);

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
    std::exit(5);
  }

  // Extract information from the file metadata
  auto sample_rate = rendered.sample_rate();
  auto num_channels = rendered.num_channels();
  auto num_samples = rendered.num_samples() < captured.num_samples()
                         ? rendered.num_samples()
                         : captured.num_samples();

  // Setup structures and classes to use EchoPathDelayEstimator
  size_t band_size = sample_rate / 16000;
  webrtc::ApmDataDumper data_dumper(0);  // basically NOP
  webrtc::EchoCanceller3Config config;
  config.delay.down_sampling_factor =
      down_sampling_factor;      // downsampling factor used to detect delay
  config.delay.num_filters = 5;  // TODO: no idea what this does

  // render_buffer[band][channel][block]
  std::vector<std::vector<std::vector<float>>> render_buffer(
      band_size, std::vector<std::vector<float>>(
                     rendered.num_channels(), std::vector<float>(webrtc::kBlockSize)));
  // capture_buffer[channel][block]
  std::vector<std::vector<float>> capture_buffer(
      captured.num_channels(), std::vector<float>(webrtc::kBlockSize));

  // Render delay buffer required to create downsampled render buffer
  std::unique_ptr<webrtc::RenderDelayBuffer> render_delay_buffer(
      webrtc::RenderDelayBuffer::Create(config, sample_rate, num_channels));

  // Actual estimator object
  webrtc::EchoPathDelayEstimator estimator(&data_dumper, config,
                                           captured.num_channels());

  spdlog::debug("---- Delay estimation start ----");
  spdlog::debug("Using the following settings:");
  spdlog::debug("  - Down sampling factor: {}",
                config.delay.down_sampling_factor);
  spdlog::debug("  - Delay filters: {}", config.delay.num_filters);

  // Loop through the entire sample to find the best delay value
  absl::optional<webrtc::DelayEstimate> estimated_delay;
  for (int i = 0; i < num_samples / webrtc::kBlockSize; i++) {
    rendered.ReadSamples(webrtc::kBlockSize, render_buffer[0][0].data());
    captured.ReadSamples(webrtc::kBlockSize, capture_buffer[0].data());

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
        spdlog::debug("Delay estimation updated at trial #{}", i);
        spdlog::debug("  - Delay: {} sample(s)", maybe_estimated_delay->delay);
        spdlog::debug("  - Quality: {}",
                      maybe_estimated_delay->quality ==
                              webrtc::DelayEstimate::Quality::kRefined
                          ? "Refined"
                          : "Coarse");
      }

      estimated_delay = maybe_estimated_delay;
    }
  }

  spdlog::debug("---- Delay estimation complete ----");

  if (estimated_delay) {
    auto delay_samples = estimated_delay->delay;
    if (brief_output)
      std::cout << delay_samples << std::endl;
    else
      spdlog::info("Estimated delay: {} sample(s) (around {}ms).",
                   delay_samples, delay_samples * 1000 / sample_rate);
  } else {
    spdlog::critical("Delay estimate is unavailable.");
  }

  return 0;
}
