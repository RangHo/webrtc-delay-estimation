#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

#include "cxxopts.hpp"

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
  // Path to WAV input files
  std::string render_filename, capture_filename;

  // Arguments used when recognizing delay
  size_t down_sampling_factor;

  // Whether the output should be brief
  bool verbose_output = false;

  // Parse command line arguments
  try {
    // clang-format butchers readability when defining options so it is better
    // to just ignore the 80-column limit
    // clang-format off
    cxxopts::Options options(argv[0], "Delay estimation algorithm extracted from WebRTC library.");
    options.add_options()
      ("h,help", "Display help message.")
      ("v,verbose", "Makes this program talk more.",
          cxxopts::value(verbose_output))
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
      std::cerr << "USAGE: " << argv[0] << " <render> <capture>" << std::endl
                << "Try '" << argv[0] << " --help' for more information."
                << std::endl;
      std::exit(2);
    }

  } catch (const cxxopts::OptionException& e) {
    std::cerr << "Unable to parse options: " << e.what() << std::endl;
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

  // Construct WAV reader from filenames provided
  webrtc::WavReader rendered(render_filename);
  webrtc::WavReader captured(capture_filename);

  // Some debug infomration about each input files
  if (verbose_output)
    std::cout << "Render file information:" << std::endl
              << "  sample rate: " << rendered.sample_rate() << std::endl
              << "  number of channels: " << rendered.num_channels()
              << std::endl
              << "  number of samples: " << rendered.num_samples() << std::endl
              << "Capture file information:" << std::endl
              << "  sample rate: " << captured.sample_rate() << std::endl
              << "  number of channels: " << captured.num_channels()
              << std::endl
              << "  number of samples: " << captured.num_samples() << std::endl;

  // If the files have different sampling rate or different amount of channels,
  if (rendered.sample_rate() != captured.sample_rate() ||
      rendered.num_channels() != captured.num_channels()) {
    std::cerr << "Render and capture files are incompatible. Cannot proceed."
              << std::endl;
    if (verbose_output)
      std::cerr << "  Render file: " << rendered.num_channels() << "ch "
                << rendered.sample_rate() << " Hz" << std::endl
                << "  Render file: " << captured.num_channels() << "ch "
                << captured.sample_rate() << " Hz" << std::endl;
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
      down_sampling_factor;  // downsampling factor used to detect delay
  config.delay.num_filters = 5;  // TODO: no idea what this does

  // render_buffer[band][channel][block]
  std::vector<std::vector<std::vector<float>>> render_buffer(
      band_size,
      std::vector<std::vector<float>>(rendered.num_channels(),
                                      std::vector<float>(webrtc::kBlockSize)));
  // capture_buffer[channel][block]
  std::vector<std::vector<float>> capture_buffer(
      captured.num_channels(), std::vector<float>(webrtc::kBlockSize));

  // Render delay buffer required to create downsampled render buffer
  std::unique_ptr<webrtc::RenderDelayBuffer> render_delay_buffer(
      webrtc::RenderDelayBuffer::Create(config, sample_rate, num_channels));

  // Actual estimator object
  webrtc::EchoPathDelayEstimator estimator(&data_dumper, config,
                                           captured.num_channels());

  if (verbose_output)
    std::cout << "Using the following settings:" << std::endl
              << "  - Down sampling factor: "
              << config.delay.down_sampling_factor << std::endl
              << "  - Delay filters: " << config.delay.num_filters << std::endl;

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
      if (verbose_output) {
        if (!estimated_delay) {
          std::cout << "First delay estimation appeared at trial #" << i << ": "
                    << maybe_estimated_delay->delay << " sample(s)"
                    << std::endl;
        } else if (estimated_delay->delay != maybe_estimated_delay->delay) {
          std::cout << (maybe_estimated_delay->quality ==
                                webrtc::DelayEstimate::Quality::kRefined
                            ? "Refined"
                            : "Coarse")
                    << " delay estimation updated at trial #" << i << ": "
                    << maybe_estimated_delay->delay << " sample(s)"
                    << std::endl;
        }

        estimated_delay = maybe_estimated_delay;
      }
    }
  }

  if (estimated_delay) {
    auto delay_samples = estimated_delay->delay;
    if (verbose_output)
      std::cout << "Estimated delay: " << delay_samples << " sample(s) (around "
                << delay_samples * 1000 / sample_rate << "ms)." << std::endl;
    else
      std::cout << delay_samples << std::endl;
  } else {
    if (verbose_output)
      std::cerr << "Delay estimate is not available." << std::endl;

    std::exit(1);
  }

  return 0;
}
