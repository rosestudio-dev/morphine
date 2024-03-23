cmake_minimum_required(VERSION 3.25)

project(morphine-app C)

set(CMAKE_C_FLAGS_DEBUG "-g ${SANITIZERS}")

add_executable(morphine-app
    main.c
    allocator.c allocator.h
    millis.c millis.h
    parseargs.c parseargs.h
    execute.c execute.h
    human.c human.h
)

target_link_libraries(
    morphine-app
    PRIVATE morphine
)


