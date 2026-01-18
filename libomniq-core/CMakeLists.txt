cmake_minimum_required(VERSION 3.16)
project(OmniQ-Core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find required packages
find_package(Eigen3 REQUIRED)
find_package(OpenMP)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Fetch pybind11
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Source files
set(OMNIQ_CORE_SOURCES
    src/common/Circuit.cpp
    src/common/Statevector.cpp
    src/common/DensityMatrix.cpp
    src/common/Operators.cpp
    src/common/QuantumStates.cpp
    src/modules/algorithms/Grovers.cpp
    src/modules/algorithms/QPE.cpp
    src/modules/algorithms/Shors.cpp
    src/modules/algorithms/HHL.cpp
    src/simulators/cpu/StatevectorSimulator.cpp
    src/simulators/cpu/DensityMatrixSimulator.cpp
    src/transpiler/Transpiler.cpp
    src/transpiler/GateDecomposer.cpp
    src/transpiler/LayoutEngine.cpp
    src/transpiler/PassManager.cpp
)

# Create shared library
add_library(omniq-core SHARED ${OMNIQ_CORE_SOURCES})

# Link libraries
target_link_libraries(omniq-core PUBLIC Eigen3::Eigen)
if(OpenMP_CXX_FOUND)
    target_link_libraries(omniq-core PRIVATE OpenMP::OpenMP_CXX)
endif()

# Set properties
set_target_properties(omniq-core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/omniq/*.h"
)

# Install rules
install(TARGETS omniq-core
    EXPORT OmniQTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/omniq
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/omniq/
    DESTINATION include/omniq
    FILES_MATCHING PATTERN "*.h"
)

# Export targets
install(EXPORT OmniQTargets
    FILE OmniQTargets.cmake
    NAMESPACE OmniQ::
    DESTINATION lib/cmake/OmniQ
)

# Create config file (simplified)
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/OmniQConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OmniQConfig.cmake
    INSTALL_DESTINATION lib/cmake/OmniQ
)

# Create static library as well
add_library(omniq-core-static STATIC ${OMNIQ_CORE_SOURCES})
set_target_properties(omniq-core-static PROPERTIES
    OUTPUT_NAME omniq-core
    VERSION ${PROJECT_VERSION}
)

target_link_libraries(omniq-core-static PUBLIC Eigen3::Eigen)
if(OpenMP_CXX_FOUND)
    target_link_libraries(omniq-core-static PRIVATE OpenMP::OpenMP_CXX)
endif()

# Python Bindings
pybind11_add_module(_omniq_core src/bindings.cpp)
target_link_libraries(_omniq_core PRIVATE omniq-core-static)
target_compile_definitions(_omniq_core PRIVATE VERSION_INFO="${PROJECT_VERSION}")

# Testing (commented out for now)
# enable_testing()
# find_package(GTest QUIET)
# if(GTest_FOUND)
#     add_executable(test_omniq_core
#         tests/test_core.cpp
#         tests/test_algorithms.cpp
#     )
#     target_link_libraries(test_omniq_core PRIVATE
#         omniq-core
#         GTest::gtest
#         GTest::gtest_main
#     )
#     add_test(NAME OmniQCoreTests COMMAND test_omniq_core)
# endif()

# Example executable (commented out for now)
# add_executable(omniq_example
#     examples/main.cpp
# )
# target_link_libraries(omniq_example PRIVATE omniq-core)

# Documentation (commented out for now)
# find_package(Doxygen QUIET)
# if(Doxygen_FOUND)
#     set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
#     set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
#     
#     configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
#     
#     add_custom_target(docs ALL
#         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         COMMENT "Generating API documentation with Doxygen"
#         VERBATIM
#     )
# endif() 