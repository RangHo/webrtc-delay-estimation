# Test sources
add_executable (webrtc-delay-estimation-tests
    "main.cc"
    "test_tools.cc"
    "test_tools.h"

    # Test files
    "random_delay_estimation_test.cc"
    "random_delay_estimation_header_test.cc"
)
target_include_directories (webrtc-delay-estimation-tests PRIVATE
    "../src"
)

# Add Catch2 for testing
find_library (CATCH2 Catch2::Catch2 REQUIRED)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/external/catch2/contrib/")
include (CTest)
include (Catch)
catch_discover_tests (webrtc-delay-estimation-tests)

# Add test dependencies
add_dependencies (webrtc-delay-estimation-tests webrtc-delay-estimation)
find_library (ABSL_STRINGS absl::strings REQUIRED)
find_library (ABSL_TYPES absl::span REQUIRED)
target_link_libraries (webrtc-delay-estimation-tests PRIVATE
    Catch2::Catch2

    webrtc-delay-estimation
    
    absl::strings
    absl::span
)

