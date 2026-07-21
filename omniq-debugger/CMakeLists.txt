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
    src/widgets/HilbertSpaceWidget.cpp
    src/widgets/ClickableLabel.cpp
)

add_executable(omniq-debugger MACOSX_BUNDLE ${DEBUGGER_SOURCES})


find_package(Eigen3 REQUIRED)

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    set(OPENGL_LIBRARIES "-framework OpenGL")
    
    # Create dummy AGL Mach-O dylib to satisfy legacy Qt6 macOS framework dependency
    set(DUMMY_AGL_DIR "${CMAKE_BINARY_DIR}/dummy_frameworks")
    file(MAKE_DIRECTORY "${DUMMY_AGL_DIR}/AGL.framework")
    file(WRITE "${CMAKE_BINARY_DIR}/dummy_agl.c" "void dummy_agl(void) {}\n")
    execute_process(COMMAND clang -dynamiclib "${CMAKE_BINARY_DIR}/dummy_agl.c" -o "${DUMMY_AGL_DIR}/AGL.framework/AGL")
    target_link_options(omniq-debugger PRIVATE "-F${DUMMY_AGL_DIR}")

    # Strip AGL from Qt6 targets if present
    foreach(qt_tgt Qt6::Gui Qt6::OpenGL Qt6::OpenGLWidgets)
        if(TARGET ${qt_tgt})
            get_target_property(libs ${qt_tgt} INTERFACE_LINK_LIBRARIES)
            if(libs)
                string(REPLACE "-framework AGL" "" libs "${libs}")
                string(REPLACE "/System/Library/Frameworks/AGL.framework" "" libs "${libs}")
                set_target_properties(${qt_tgt} PROPERTIES INTERFACE_LINK_LIBRARIES "${libs}")
            endif()
        endif()
    endforeach()
else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBRARIES OpenGL::GL)
endif()

target_link_libraries(omniq-debugger PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Eigen3::Eigen
    ${OPENGL_LIBRARIES}
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
    BUNDLE DESTINATION .
) 