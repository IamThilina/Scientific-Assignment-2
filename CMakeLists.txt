cmake_minimum_required(VERSION 3.6)
project(assignment_2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c sparseMatrixAddition.c)
add_executable(main main.c)
add_executable(sparseMatrixAddition sparseMatrixAddition.c)