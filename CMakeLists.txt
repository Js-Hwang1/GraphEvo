cmake_minimum_required(VERSION 3.10)
project(GraphEVO CXX)

# Set C++17 standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wno-unused-variable -Wno-unused-but-set-variable")

# --- Eigen Configuration ---
# Check if vendored Eigen exists in third_party/eigen.
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/eigen/Eigen/Core")
    message(STATUS "Using vendored Eigen from third_party/eigen")
    set(Eigen3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/third_party/eigen")
else()
    # Try to find Eigen3 using CMake's find_package.
    find_package(Eigen3 3.3 QUIET NO_MODULE)
    if (Eigen3_FOUND)
        message(STATUS "Eigen3 found: ${Eigen3_INCLUDE_DIRS}")
    else()
        message(WARNING "Eigen3 not found via find_package; falling back to EIGEN_INCLUDE variable.")
        if(NOT DEFINED EIGEN_INCLUDE)
            # Default path for Eigen installed via Homebrew on macOS.
            set(EIGEN_INCLUDE "/opt/homebrew/opt/eigen/include/eigen3")
        endif()
        set(Eigen3_INCLUDE_DIRS ${EIGEN_INCLUDE})
    endif()
endif()

# Set the Spectra include directory. Assume it's vendored as a submodule.
if(NOT DEFINED SPECTRA_INCLUDE)
    set(SPECTRA_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/spectra/include")
endif()

# Find OpenMP.
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Include directories for project headers, Eigen3, and Spectra.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${Eigen3_INCLUDE_DIRS})
include_directories(${SPECTRA_INCLUDE})

# Collect all source files from the src/ directory.
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Create the executable.
add_executable(GraphEVO ${SOURCES})

# Link with OpenMP if available.
if(OpenMP_CXX_FOUND)
    target_link_libraries(GraphEVO OpenMP::OpenMP_CXX)
endif()
