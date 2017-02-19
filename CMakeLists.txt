cmake_minimum_required(VERSION 3.6)
project(assignment_2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c sparseMatrixAddition.c)
add_executable(main main.c)
add_executable(sparseMatrixAddition sparseMatrixAddition.c)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
endif()