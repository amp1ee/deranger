cmake_minimum_required(VERSION 3.15)

PROJECT(Deranger
LANGUAGES CXX C
VERSION 0.8.7
)

# for clang-tidy(this enable to find system header files).
if(CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Enable JUCE. Do not use find_package to prevent from mix up with one globally installed.
set(JUCE_ROOT "${CMAKE_SOURCE_DIR}/lib/JUCE")
add_subdirectory(${JUCE_ROOT})

set(SIGNALSMITH_ROOT "${CMAKE_SOURCE_DIR}/lib/SignalSmith")
include_directories(Deranger
    PRIVATE
        ${SIGNALSMITH_ROOT})

add_subdirectory(src)
