cmake_minimum_required(VERSION 3.16)
project(OmniQ-C VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Eigen3
find_package(Eigen3 REQUIRED)

# Source files
set(C_SOURCES
    src/omniq_c.cpp
)

# Create shared library
add_library(omniq-c SHARED ${C_SOURCES})

# Set include directories
target_include_directories(omniq-c PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(omniq-c PRIVATE
    Eigen3::Eigen
)

# Set properties
set_target_properties(omniq-c PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/omniq_c.h
)

# Install rules
install(TARGETS omniq-c
    EXPORT OmniQ-CTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Install headers
install(FILES include/omniq_c.h
    DESTINATION include
)

# Export targets
install(EXPORT OmniQ-CTargets
    FILE OmniQ-CTargets.cmake
    NAMESPACE OmniQ::
    DESTINATION lib/cmake/OmniQ-C
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/OmniQ-CConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/OmniQ-CConfig.cmake"
    INSTALL_DESTINATION lib/cmake/OmniQ-C
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/OmniQ-CConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/OmniQ-CConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/OmniQ-CConfigVersion.cmake"
    DESTINATION lib/cmake/OmniQ-C
) 