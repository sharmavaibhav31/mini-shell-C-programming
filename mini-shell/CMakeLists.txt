cmake_minimum_required(VERSION 3.10)
project(OfflineAssistShell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")

# Shell executable
add_executable(myshell 
    main.c 
    shell.c
)

# AI worker executable (basic version without llama.cpp first)
add_executable(ai_worker 
    ai_worker.c
    sandbox.c
    seccomp_filter.c
)

# Optional: link libcap if available
find_library(CAP_LIB cap)
if(CAP_LIB)
    target_link_libraries(ai_worker ${CAP_LIB})
    target_compile_definitions(ai_worker PRIVATE HAVE_LIBCAP)
endif()

# Installation
install(TARGETS myshell ai_worker DESTINATION bin)
