cmake_minimum_required (VERSION 3.12)

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

# Platform-specific build arguments
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions (
        "WEBRTC_WIN"
        "USE_AURA=1"
        "CR_CLANG_REVISION=\"llvmorg-12-init-12923-g6ee22ca6-1\""
        "_HAS_NODISCARD"
        "_LIBCPP_ABI_UNSTABLE"
        "_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS"
        "_LIBCPP_ENABLE_NODISCARD"
        "_LIBCPP_DEBUG=0"
        "_LIBCPP_NO_AUTO_LINK"
        "__STD_C"
        "_CRT_RAND_S"
        "_CRT_SECURE_NO_DEPRECATE"
        "_SCL_SECURE_NO_DEPRECATE"
        "_ATL_NO_OPENGL"
        "_WINDOWS"
        "CERT_CHAIN_PARA_HAS_EXTRA_FIELDS"
        "PSAPI_VERSION=2"
        "WIN32"
        "_SECURE_ATL"
        "WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP"
        "WIN32_LEAN_AND_MEAN"
        "NOMINMAX"
        "_UNICODE"
        "UNICODE"
        "NTDDI_VERSION=NTDDI_WIN10_VB"
        "_WIN32_WINNT=0x0A00"
        "WINVER=0x0A00"
        "_DEBUG"
        "DYNAMIC_ANNOTATIONS_ENABLED=1"
        "WEBRTC_ENABLE_PROTOBUF=1"
        "WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE"
        "RTC_ENABLE_VP9"
        "HAVE_SCTP"
        "WEBRTC_LIBRARY_IMPL"
        "WEBRTC_ENABLE_AVX2"
        "WEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=1"
        "WEBRTC_WIN"
        "ABSL_ALLOCATOR_NOTHROW=1"
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_compile_definitions (
        "WEBRTC_POSIX"
        "WEBRTC_MAC"
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_compile_definitions (
        "WEBRTC_POSIX"
        "WEBRTC_LINUX"
    )
    add_compile_options(-march=skylake-avx512)
else ()
    message (FATAL_ERROR "Unsupported platform detected!")
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
    "src/field_trial.cc"
)

# Platform-specific source code
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources (webrtc-delay-estimation PRIVATE "src/cpu_features_linux.cc")
endif ()

target_link_libraries (webrtc-delay-estimation absl::strings)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Windows Multimedia Library is required for Windows
    target_link_libraries (webrtc-delay-estimation winmm)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # pthread library is required for Linux
    target_link_libraries (webrtc-delay-estimation pthread)
endif ()

