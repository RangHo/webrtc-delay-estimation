cmake_minimum_required (VERSION 3.8)

project ("webrtc-delay-estimation")

# Set default build type
set (default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE)
    message (STATUS "Setting build type to ${default_build_type} as none was specified.")
    set (CMAKE_BUILD_TYPE "${default_build_type}")
endif ()

# Set C++ standard to use
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED true)

# Define debug/release constants
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DWEBRTC_APM_DEBUG_DUMP=1")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWEBRTC_APM_DEBUG_DUMP=0")

# Platform-specific constants
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_WIN")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_POSIX")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_MAC")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_POSIX")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX -lpthread -march=skylake-avx512")
endif ()


# Add Abseil library
add_subdirectory ("external/abseil-cpp")
include_directories ("external/abseil-cpp")

# Executable sources
add_executable (webrtc-delay-estimation

    # entrypoint
    "src/main.cc"

    # api/
    "src/arraysize.h"

    # api/audio/
    "src/echo_canceller3_config.cc"
    "src/echo_canceller3_config.h"

    # common_audio/
    "src/wav_file.cc"
    "src/wav_file.h"
    "src/wav_header.cc"
    "src/wav_header.h"

    # common_audio/include/
    "src/audio_util.h"
    
    # modules/audio_processing/aec3/
    "src/aec3_common.cc"
    "src/aec3_common.h"
    "src/alignment_mixer.cc"
    "src/alignment_mixer.h"
    "src/clockdrift_detector.cc"
    "src/clockdrift_detector.h"
    "src/decimator.cc"
    "src/decimator.h"
    "src/delay_estimate.h"
    "src/downsampled_render_buffer.cc"
    "src/downsampled_render_buffer.h"
    "src/echo_path_delay_estimator.cc"
    "src/echo_path_delay_estimator.h"
    "src/matched_filter.cc"
    "src/matched_filter.h"
    "src/matched_filter_avx2.cc"
    "src/matched_filter_lag_aggregator.cc"
    "src/matched_filter_lag_aggregator.h"

    # modules/audio_processing/logging/
    "src/apm_data_dumper.cc"
    "src/apm_data_dumper.h"

    # modules/audio_processing/utility/
    "src/cascaded_biquad_filter.cc"
    "src/cascaded_biquad_filter.h"

    # rtc_base/
    "src/array_view.h"
    "src/checks.cc"
    "src/checks.h"
    "src/constructor_magic.h"
    "src/deprecation.h"
    "src/logging.cc"
    "src/logging.h"
    "src/platform_thread_types.cc"
    "src/platform_thread_types.h"
    "src/sanitizer.h"
    "src/string_utils.cc"
    "src/string_utils.h"
    "src/thread_annotations.h"
    "src/time_utils.cc"
    "src/time_utils.h"
    "src/type_traits.h"

    # rtc_base/numerics/
    "src/safe_compare.h"
    "src/safe_conversions.h"
    "src/safe_conversions_impl.h"
    "src/safe_minmax.h"

    # rtc_base/strings/
    "src/string_builder.cc"
    "src/string_builder.h"
    "src/string_encode.cc"
    "src/string_encode.h"
    "src/string_to_number.cc"
    "src/string_to_number.h"

    # rtc_base/synchronization/
    "src/mutex.cc"
    "src/mutex.h"
    "src/mutex_abseil.h"
    "src/mutex_critical_section.h"
    "src/mutex_pthread.h"
    "src/yield.cc"
    "src/yield.h"

    # rtc_base/system/
    "src/arch.h"
    "src/file_wrapper.cc"
    "src/file_wrapper.h"
    "src/inline.h"
    "src/rtc_export.h"
    "src/unused.h"

    # system_wrappers/include/
    "src/cpu_features_wrapper.h"
    "src/field_trial.h"

    # system_wrappers/source/
    "src/cpu_features.cc"
    #"src/cpu_features_android.cc"
    "src/cpu_features_linux.cc"
    "src/field_trial.cc"
)

target_link_libraries (webrtc-delay-estimation absl::strings)

