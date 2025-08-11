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

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/libomniq-core/include)

# Source files for the core library
set(CORE_SOURCES
    libomniq-core/src/common/Statevector.cpp
    libomniq-core/src/common/DensityMatrix.cpp
    libomniq-core/src/common/Circuit.cpp
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
target_link_libraries(omniq-core PUBLIC Threads::Threads)

# Create the test executables
add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE omniq-core)

add_executable(test_algorithms test_algorithms.cpp)
target_link_libraries(test_algorithms PRIVATE omniq-core)

# Create the main executable
add_executable(omniq main)
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

# Enable testing
enable_testing()

# Add tests
add_test(NAME CoreTest COMMAND test_core)
add_test(NAME AlgorithmsTest COMMAND test_algorithms)

# Print configuration summary
message(STATUS "OmniQ Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}") 