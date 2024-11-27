cmake_minimum_required(VERSION 3.10)
project(ModernCppStarter)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Find clang-tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    # set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
endif()

# clang-format
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  set(CMAKE_CXX_CLANG_FORMAT ${CLANG_FORMAT})
endif()

# Install gtest and gmock
include(FetchContent)
FetchContent_Declare(
  googletest
  URL  https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

# Include the directories
include_directories(${PROJECT_SOURCE_DIR}/3rd)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

# Set output directories for different build types
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# Define cache flags for compilation 
set(DELELGATION_FLAGS "" CACHE STRING "Flag for compiling the delegation sketch")
set(FREQUENCY_ESTIMATOR_FLAGS "" CACHE STRING "Flag for compiling the frequency estimator and related files")



# Add subdirectories
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)
