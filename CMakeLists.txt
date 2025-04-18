cmake_minimum_required(VERSION 3.15)

project(calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
add_compile_options(-Wall -Wpedantic -Wextra -Wundef -Wwrite-strings -Wredundant-decls -Wdisabled-optimization 
                -Wdouble-promotion -Wmissing-declarations -Wconversion -Wstrict-overflow=2 -fstack-protector-all -Wvla -O3)

# Specify header and source files
set(CALCULATOR_HEADER_FILES
    src/memory/memory.hpp
    src/lexer/lexer.hpp
    src/parser/parser.hpp
)

set(CALCULATOR_SOURCE_FILES
    src/main.cpp

    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/parser/expr.cpp

    src/eval/eval.cpp
)

# Create executable
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(calculator ${CALCULATOR_HEADER_FILES} ${CALCULATOR_SOURCE_FILES})
add_link_options(-lstdc++)
