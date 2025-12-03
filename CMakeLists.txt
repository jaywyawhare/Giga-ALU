cmake_minimum_required(VERSION 3.15)

project(giga_alu
    VERSION 0.1.0
    DESCRIPTION "Giga-ALU: a 4-bit ALU virtual machine with custom ISA"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Main executable
add_executable(alu_vm
    src/main.c
    src/alu/alu.c
    src/vm/vm.c)

target_include_directories(alu_vm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(alu_vm PRIVATE c_std_17)

# ALU tests
add_executable(alu_tests
    src/alu/alu.c
    tests/alu_tests.c)

target_include_directories(alu_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(alu_tests PRIVATE c_std_17)

# Lexer tests
add_executable(lexer_tests
    src/lexer/lexer.c
    tests/lexer_tests.c)

target_include_directories(lexer_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(lexer_tests PRIVATE c_std_17)

# Parser tests
add_executable(parser_tests
    src/lexer/lexer.c
    src/parser/parser.c
    tests/parser_tests.c)

target_include_directories(parser_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(parser_tests PRIVATE c_std_17)

# VM tests
add_executable(vm_tests
    src/vm/vm.c
    tests/vm_tests.c)

target_include_directories(vm_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(vm_tests PRIVATE c_std_17)
