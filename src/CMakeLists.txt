# Export WebRTC portion as a library
add_library (webrtc-delay-estimation

    # api/
    "arraysize.h"

    # api/audio/
    "echo_canceller3_config.cc"
    "echo_canceller3_config.h"

    # common_audio/
    "wav_file.cc"
    "wav_file.h"
    "wav_header.cc"
    "wav_header.h"

    # common_audio/include/
    "audio_util.h"

    # common_audio/third_party/ooura/fft_size_128
    "ooura_fft.cc"
    "ooura_fft.h"
    "ooura_fft_sse2.cc"
    "ooura_fft_tables_common.h"
    "ooura_fft_tables_neon_sse2.h"
    
    # modules/audio_processing/aec3/
    "aec3_common.cc"
    "aec3_common.h"
    "aec3_fft.cc"
    "aec3_fft.h"
    "alignment_mixer.cc"
    "alignment_mixer.h"
    "block_buffer.cc"
    "block_buffer.h"
    "clockdrift_detector.cc"
    "clockdrift_detector.h"
    "decimator.cc"
    "decimator.h"
    "delay_estimate.h"
    "downsampled_render_buffer.cc"
    "downsampled_render_buffer.h"
    "echo_path_delay_estimator.cc"
    "echo_path_delay_estimator.h"
    "fft_buffer.cc"
    "fft_buffer.h"
    "fft_data.h"
    "fft_data_avx2.cc"
    "matched_filter.cc"
    "matched_filter.h"
    "matched_filter_avx2.cc"
    "matched_filter_lag_aggregator.cc"
    "matched_filter_lag_aggregator.h"
    "render_buffer.cc"
    "render_buffer.h"
    "render_delay_buffer.cc"
    "render_delay_buffer.h"
    "spectrum_buffer.cc"
    "spectrum_buffer.h"

    # modules/audio_processing/logging/
    "apm_data_dumper.cc"
    "apm_data_dumper.h"

    # modules/audio_processing/utility/
    "cascaded_biquad_filter.cc"
    "cascaded_biquad_filter.h"

    # rtc_base/
    "array_view.h"
    "atomic_ops.h"
    "checks.cc"
    "checks.h"
    "constructor_magic.h"
    "deprecation.h"
    "logging.cc"
    "logging.h"
    "platform_thread_types.cc"
    "platform_thread_types.h"
    "sanitizer.h"
    "string_utils.cc"
    "string_utils.h"
    "thread_annotations.h"
    "time_utils.cc"
    "time_utils.h"
    "type_traits.h"

    # rtc_base/numerics/
    "safe_compare.h"
    "safe_conversions.h"
    "safe_conversions_impl.h"
    "safe_minmax.h"

    # rtc_base/strings/
    "string_builder.cc"
    "string_builder.h"
    "string_encode.cc"
    "string_encode.h"
    "string_to_number.cc"
    "string_to_number.h"

    # rtc_base/synchronization/
    "mutex.cc"
    "mutex.h"
    "mutex_abseil.h"
    "mutex_critical_section.h"
    "mutex_pthread.h"
    "yield.cc"
    "yield.h"

    # rtc_base/system/
    "arch.h"
    "file_wrapper.cc"
    "file_wrapper.h"
    "inline.h"
    "rtc_export.h"
    "unused.h"

    # system_wrappers/include/
    "cpu_features_wrapper.h"
    "field_trial.h"

    # system_wrappers/source/
    "cpu_features.cc"
    "field_trial.cc"
)

# Platform-specific source code
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources (webrtc-delay-estimation PRIVATE "cpu_features_linux.cc")
endif ()

# Add main library dependencies
add_dependencies (webrtc-delay-estimation
    absl::strings
)
find_library (ABSL_STRINGS absl::strings REQUIRED)
target_link_libraries (webrtc-delay-estimation PRIVATE
    absl::strings
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Windows Multimedia Library is required for Windows
    target_link_libraries (webrtc-delay-estimation PRIVATE winmm)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # pthread library is required for Linux
    target_link_libraries (webrtc-delay-estimation PRIVATE pthread)
endif ()

# Standalone executable build
add_executable (delay-estimator
    "main.cc"
)

# Add executable dependencies
add_dependencies (delay-estimator
    webrtc-delay-estimation

    absl::strings
    cxxopts
    spdlog::spdlog
)
find_library (ABSL_STRINGS absl::strings REQUIRED)
find_library (CXXOPTS cxxopts REQUIRED)
target_link_libraries(delay-estimator PRIVATE
    webrtc-delay-estimation

    absl::strings
    cxxopts
)

