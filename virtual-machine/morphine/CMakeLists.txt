cmake_minimum_required(VERSION 3.25)
project(morphine-lib C)

set(CMAKE_C_STANDARD 99)

set(COMMON_FLAGS "-fno-common -fwrapv")
set(WARNINGS "-Wall -Wextra -Werror -Wno-error=conversion -Wdouble-promotion -Wshadow -Wformat -Wstrict-prototypes -Wunused-parameter -Wconversion -Wredundant-decls -Wmissing-declarations -Wvla -Wmissing-prototypes -Wcast-qual -Wcast-align")

set(CMAKE_C_FLAGS_DEBUG "-g ${WARNINGS} ${COMMON_FLAGS} ${SANITIZERS}")
set(CMAKE_C_FLAGS_RELEASE "-O2 -march=native ${COMMON_FLAGS} ")

file(GLOB_RECURSE morphine_source
    "core/*.c"
    "api/*.c"
    "auxiliary/*.c"
    "libs/*.c"
    "platform/*.c"
    "object/*.c"
    "gc/*.c"
    "stack/*.c"
)

include_directories("headers" "include")

add_library(morphine STATIC ${morphine_source})
target_link_libraries(morphine m)
target_include_directories(
    morphine
    PUBLIC "include"
    PRIVATE "headers"
)

add_library(morphine-debug STATIC ${morphine_source})
target_link_libraries(morphine-debug m)
target_include_directories(
    morphine-debug
    PUBLIC "include"
    PUBLIC "headers"
)
target_compile_definitions(
    morphine-debug
    PUBLIC MORPHINE_ENABLE_DEBUGGER
)
