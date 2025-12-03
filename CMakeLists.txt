cmake_minimum_required(VERSION 3.15)

project(giga_alu
    VERSION 0.1.0
    DESCRIPTION "Giga-ALU: a 4-bit ALU virtual machine with custom ISA"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(alu_vm
    src/main.c)

target_compile_features(alu_vm PRIVATE c_std_17)

