cmake_minimum_required(VERSION 3.25)

project(morphine-app C)

set(CMAKE_C_FLAGS_DEBUG "-g ${SANITIZERS}")

include_directories(include)

add_executable(morphine-app
    main.c
    allocator.c
    millis.c
    parseargs.c
    execute.c
    loaders.c
    human.c
    dlibcompiler.c
    userdata/readfile.c
    userdata/tbcfile.c
)

target_link_libraries(
    morphine-app
    PRIVATE morphine
    PRIVATE libmorphinecompiler
)


