cmake_minimum_required(VERSION 3.16)
project(omniq-qos VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libomniq-core/include
)

# 1. QMMU (Quantum Memory Management Unit) Library
add_library(omniq-qmmu STATIC
    src/qmmu/QMMU.cpp
    src/driver/VirtualQPUDriver.cpp
)
target_link_libraries(omniq-qmmu PUBLIC omniq-core Threads::Threads)

# 2. Client Library (libomniq-qpu)
add_library(omniq-qpu SHARED
    src/lib/libomniq_qpu.cpp
)
target_link_libraries(omniq-qpu PRIVATE omniq-qmmu omniq-core Threads::Threads)
set_target_properties(omniq-qpu PROPERTIES
    OUTPUT_NAME "omniq-qpu"
    VERSION 0.1.0
    SOVERSION 1
)

# 3. Daemon (omniqd)
add_executable(omniqd
    src/daemon/omniqd.cpp
)
target_link_libraries(omniqd PRIVATE omniq-qmmu omniq-qpu omniq-core Threads::Threads)

# 4. POSIX Example Client (C)
add_executable(posix_qpu_client
    examples/posix_qpu_client.c
)
target_link_libraries(posix_qpu_client PRIVATE omniq-qpu Threads::Threads)
