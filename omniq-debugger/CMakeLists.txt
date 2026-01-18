cmake_minimum_required(VERSION 3.16)
project(OmniQ-Debugger VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "/opt/homebrew")

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/../libomniq-core/include)

# Source files
set(DEBUGGER_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/CoreInterface.cpp
    src/QuantumBackend.cpp
    src/widgets/CircuitView.cpp
    src/widgets/QuantumStateViewer.cpp
    src/widgets/QubitViewer.cpp
    src/widgets/CircuitBuilder.cpp
    src/widgets/BlochSphereWidget.cpp
    src/widgets/EntanglementGraphWidget.cpp
)

add_executable(omniq-debugger ${DEBUGGER_SOURCES})


find_package(Eigen3 REQUIRED)

target_link_libraries(omniq-debugger PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Eigen3::Eigen
    # OmniQ::omniq-core  # commented out just for now
)

target_compile_options(omniq-debugger PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

set_target_properties(omniq-debugger PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Install
install(TARGETS omniq-debugger
    RUNTIME DESTINATION bin
) 