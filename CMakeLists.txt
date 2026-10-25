cmake_minimum_required(VERSION 4.4.4)
project(mandelbrot LANGUAGES C)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make compiler output colorful
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Set project root for all submodules
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(CUDAToolkit)
if(CUDAToolkit_FOUND)
    message(STATUS "Found CUDA version: ${CUDAToolkit_VERSION}.")

    # Enable CUDA as a first-class language for .cu files
    enable_language(CUDA)
else()
    message(STATUS "CUDA not found, defaulting to CPU build.")
endif()

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Add the mandelbrot library
add_subdirectory(mandelbrot)

# Add the application code
add_subdirectory(app)
