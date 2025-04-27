cmake_minimum_required(VERSION 3.15)

PROJECT(JUCE_CMAKE_EXAMPLE
LANGUAGES CXX C
VERSION 0.0.2
)

if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "MSVC detected — forcing ClangCL for better C++ standard compliance")
    # Force use of ClangCL instead of MSVC cl.exe
    set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe" CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe" CACHE STRING "" FORCE)
endif()

# for clang-tidy(this enable to find system header files).
if(CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Enable JUCE. Do not use find_package to prevent from mix up with one globally installed.
set(JUCE_ROOT "${CMAKE_SOURCE_DIR}/lib/JUCE")
add_subdirectory(${JUCE_ROOT})

add_subdirectory(src)
