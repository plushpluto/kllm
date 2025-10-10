# CMake configuration for the KLLM Kernel Layer
# This file will define the build process for the core C/C++ components.

cmake_minimum_required(VERSION 3.10)

project(kllm_kernel C CXX)

# Set standard C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add llama.cpp as a subdirectory. This will define the `llama` and `ggml` targets.
# We disable tests and examples to speed up the build.
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "Build tests")
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
add_subdirectory(${CMAKE_SOURCE_DIR}/../externals/llama.cpp)

# Add source files for the kernel library
add_library(kllm_kernel_lib STATIC
    src/model_manager.cpp
    src/resource_allocator.cpp
)

# Include directories
target_include_directories(kllm_kernel_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/../sdk/include
)

# Add executables for testing or standalone components
add_executable(kllm_main main.cpp)
target_link_libraries(kllm_main PRIVATE kllm_kernel_lib llama ggml common)