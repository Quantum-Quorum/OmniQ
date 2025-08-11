cmake_minimum_required(VERSION 3.16)
project(OmniQ-Debugger VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 paths for Homebrew installation
set(CMAKE_PREFIX_PATH "/opt/homebrew")

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Find OmniQ core library (we'll build it locally for now)
# find_package(OmniQ REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/../libomniq-core/include)

# Source files
set(DEBUGGER_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/CoreInterface.cpp
    src/widgets/CircuitView.cpp
    src/widgets/QuantumStateViewer.cpp
    src/widgets/QubitViewer.cpp
)

# Create executable
add_executable(omniq-debugger ${DEBUGGER_SOURCES})

# Link libraries
target_link_libraries(omniq-debugger PRIVATE 
    Qt6::Core 
    Qt6::Widgets
    # OmniQ::omniq-core  # Commented out for now
)

# Set compiler flags
target_compile_options(omniq-debugger PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Enable automoc for Qt
set_target_properties(omniq-debugger PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Install
install(TARGETS omniq-debugger
    RUNTIME DESTINATION bin
) 