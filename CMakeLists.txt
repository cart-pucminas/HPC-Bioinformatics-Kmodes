cmake_minimum_required(VERSION 3.6)
project(CudaPredTIS)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    src/global.h
    src/io.c
    src/io.h
    src/kmodes.c
    src/kmodes.h
    src/kmodes_mpi.c
    src/kmodes_openmp.c
    src/main.c
    src/power.c
    src/power.h
    src/sequence.c
    src/sequence.h
    Makefile)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(kmodes ALL COMMAND make kmodes WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
