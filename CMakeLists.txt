cmake_minimum_required(VERSION 3.15)

project(giga_alu
    VERSION 0.1.0
    DESCRIPTION "Giga-ALU: a 4-bit ALU virtual machine with custom ISA"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(alu_vm
    src/main.c
    src/alu.c
    src/isa.h
    src/vm.c
    src/vm.h)

target_compile_features(alu_vm PRIVATE c_std_17)

add_executable(alu_tests
    src/alu.c
    tests/alu_tests.c)

target_include_directories(alu_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(alu_tests PRIVATE c_std_17)

add_executable(lexer_dump
    src/lexer.c
    src/lexer.h
    src/lexer_main.c)

target_include_directories(lexer_dump PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(lexer_dump PRIVATE c_std_17)

