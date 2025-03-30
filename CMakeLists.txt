cmake_minimum_required(VERSION 3.10)
project(GraphEVO CXX)

# Set the C++ standard and common compiler flags.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

# Find OpenMP.
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Find Eigen3.
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
if(Eigen3_FOUND)
    message(STATUS "Eigen3 found")
endif()

# Set the Spectra include directory (vendored as a submodule in third_party/spectra).
set(SPECTRA_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/spectra/include")

# Include directories for your project headers, Eigen, and Spectra.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${Eigen3_INCLUDE_DIRS})
include_directories(${SPECTRA_INCLUDE})

# Collect all source files from the src/ directory.
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Create the executable.
add_executable(GraphEVO ${SOURCES})

# Link OpenMP if available.
if(OpenMP_CXX_FOUND)
    target_link_libraries(GraphEVO OpenMP::OpenMP_CXX)
endif()
