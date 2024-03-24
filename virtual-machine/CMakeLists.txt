cmake_minimum_required(VERSION 3.25)
project(morphine-lang)

option(USE_SANITIZERS "Use sanitizers" OFF)

if (USE_SANITIZERS)
    set(SANITIZERS "-fsanitize=address -fsanitize=undefined")
else ()
    set(SANITIZERS "")
endif ()

include(../build/lib/dynamic/morphine-compiler-clib/libmorphinecompiler.cmake)

add_subdirectory(morphine)
add_subdirectory(application)
