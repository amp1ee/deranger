cmake_minimum_required(VERSION 3.15)

PROJECT(JUCE_CMAKE_EXAMPLE
LANGUAGES CXX C
VERSION 0.0.2
)

if(MSVC)
    add_compile_options(/w)  # Disables all warnings for MSVC
endif()

# for clang-tidy(this enable to find system header files).
if(CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Enable JUCE. Do not use find_package to prevent from mix up with one globally installed.
set(JUCE_ROOT "${CMAKE_SOURCE_DIR}/lib/JUCE")
add_subdirectory(${JUCE_ROOT})

add_subdirectory(src)
