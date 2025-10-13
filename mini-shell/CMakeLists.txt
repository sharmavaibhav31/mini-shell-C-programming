cmake_minimum_required(VERSION 3.10)
project(OfflineAssistShell C CXX)

# Basic Compilation Settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fstack-protector-strong")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fstack-protector-strong")

# Shell executable
add_executable(myshell main.c shell.c)

# --- LLAMA.CPP INTEGRATION ---
# 1. Include llama.cpp as a subdirectory (Recommended for static linking)
# This assumes the 'llama.cpp' directory is a sibling to 'build' or in the root.
add_subdirectory(llama.cpp)

# 2. AI worker executable
add_executable(ai_worker ai_worker.c sandbox.c seccomp_filter.c)

# Check if the 'llama' target was successfully added (meaning the project was found)
if(TARGET llama)
    target_compile_definitions(ai_worker PRIVATE USE_LLAMA)

    # Link directly against the targets defined by llama.cpp's CMake
    # The 'llama' target automatically brings in the 'ggml' targets and necessary includes.
    target_link_libraries(ai_worker
        PRIVATE
            llama
            ${CMAKE_THREAD_LIBS_INIT} # Use CMake's standard way to find pthread
            m
            dl
            stdc++
    )

    message(STATUS "✓✓✓ Building WITH llama.cpp support (using CMake targets) ✓✓✓")
else()
    message(WARNING "Building without llama.cpp (echo mode only). Check if llama.cpp exists.")
endif()

# --- OPTIONAL DEPENDENCIES ---
# libcap
find_library(CAP_LIB cap)
if(CAP_LIB)
    target_link_libraries(ai_worker PRIVATE ${CAP_LIB})
    target_compile_definitions(ai_worker PRIVATE HAVE_LIBCAP)
endif()

# --- INSTALLATION ---
install(TARGETS myshell ai_worker DESTINATION bin)
