cmake_minimum_required(VERSION 3.15)

project(zura2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
add_compile_options(-Wall -Wpedantic -Wextra -Wundef -Wwrite-strings -Wredundant-decls -Wdisabled-optimization 
                -Wdouble-promotion -Wmissing-declarations -Wconversion -Wstrict-overflow=2 -fstack-protector-all -Wvla -O3)

# Specify header and source files
set(ZURA2_HEADER_FILES
    src/memory/memory.hpp
    src/error/error.hpp 
    src/lexer/lexer.hpp
    src/parser/parser.hpp
)

set(ZURA2_SOURCE_FILES
    src/main.cpp

    src/error/error.cpp

    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/parser/expr.cpp
    src/parser/stmt.cpp

    src/codegen/llvm_stmt.cpp
    src/codegen/llvm_expr.cpp
    src/codegen/llvm_type.cpp
)

# Create executable
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(zura2 ${ZURA2_HEADER_FILES} ${ZURA2_SOURCE_FILES})

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

target_link_libraries(zura2 PRIVATE LLVM)

target_link_libraries(zura2 PRIVATE ${LLVM_LIBS})

add_link_options(-lstdc++)
