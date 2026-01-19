cmake_minimum_required(VERSION 3.16)
project(OmniQ VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(Threads REQUIRED)

# Find Eigen3 or fetch it
find_package(Eigen3 QUIET)
if(NOT Eigen3_FOUND)
    message(STATUS "Eigen3 not found. Fetching from source...")
    include(FetchContent)
    FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Eigen3)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/libomniq-core/include)

# Source files for the core library
set(CORE_SOURCES
    libomniq-core/src/common/Statevector.cpp
    libomniq-core/src/common/DensityMatrix.cpp
    libomniq-core/src/common/Circuit.cpp
    libomniq-core/src/common/Operators.cpp
    libomniq-core/src/common/QuantumStates.cpp
    libomniq-core/src/modules/algorithms/Grovers.cpp
    libomniq-core/src/modules/algorithms/QPE.cpp
)

# Create the core library
add_library(omniq-core STATIC ${CORE_SOURCES})
target_include_directories(omniq-core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libomniq-core/include>
    $<INSTALL_INTERFACE:include>
)

# Set compiler flags
target_compile_options(omniq-core PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link libraries
target_link_libraries(omniq-core PUBLIC Threads::Threads Eigen3::Eigen)

# Create the test executables
option(BUILD_TESTING "Build tests" OFF)

if(BUILD_TESTING)
    # Find Google Test or fetch it
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        message(STATUS "GTest not found. Fetching from source...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        FetchContent_MakeAvailable(googletest)
        include(GoogleTest)
    endif()

    # Test executables
    add_executable(test_core tests/test_core.cpp)
    target_link_libraries(test_core PRIVATE omniq-core GTest::gtest GTest::gtest_main)

    add_executable(test_algorithms test_algorithms.cpp)
    target_link_libraries(test_algorithms PRIVATE omniq-core GTest::gtest GTest::gtest_main)

    # Enable testing
    enable_testing()
    add_test(NAME CoreTest COMMAND test_core)
    add_test(NAME AlgorithmsTest COMMAND test_algorithms)
endif()

# Create the main executable
add_executable(omniq main.cpp)
target_link_libraries(omniq PRIVATE omniq-core)

# Install targets
install(TARGETS omniq-core
    EXPORT OmniQTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY libomniq-core/include/omniq
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install export
install(EXPORT OmniQTargets
    FILE OmniQTargets.cmake
    NAMESPACE OmniQ::
    DESTINATION lib/cmake/OmniQ
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    OmniQConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OmniQConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OmniQConfig.cmake
    INSTALL_DESTINATION lib/cmake/OmniQ
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OmniQConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OmniQConfigVersion.cmake
    DESTINATION lib/cmake/OmniQ
)

# Print configuration summary
message(STATUS "OmniQ Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}") 