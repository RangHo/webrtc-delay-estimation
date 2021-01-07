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
    add_compile_options(-march=native)
else ()
    message (FATAL_ERROR "Unsupported platform detected!")
endif ()

# Add dependency subdirectories
add_subdirectory ("external/abseil-cpp" EXCLUDE_FROM_ALL)
add_subdirectory ("external/catch2" EXCLUDE_FROM_ALL)
add_subdirectory ("external/cxxopts" EXCLUDE_FROM_ALL)
add_subdirectory ("external/spdlog" EXCLUDE_FROM_ALL)

# Add source directory
add_subdirectory ("src")

# Add test directory
add_subdirectory ("tests")

