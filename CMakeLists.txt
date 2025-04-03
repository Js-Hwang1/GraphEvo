cmake_minimum_required(VERSION 3.10)
project(GraphEVO CXX)

# Set C++17 standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wno-unused-variable -Wno-unused-but-set-variable")

# --- Eigen Configuration ---
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/eigen/Eigen/Core")
    message(STATUS "Using vendored Eigen from third_party/eigen")
    set(Eigen3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/third_party/eigen")
else()
    find_package(Eigen3 3.3 QUIET NO_MODULE)
    if (Eigen3_FOUND)
        message(STATUS "Eigen3 found: ${Eigen3_INCLUDE_DIRS}")
    else()
        message(WARNING "Eigen3 not found via find_package; falling back to EIGEN_INCLUDE variable.")
        if(NOT DEFINED EIGEN_INCLUDE)
            set(EIGEN_INCLUDE "/opt/homebrew/opt/eigen/include/eigen3")
        endif()
        set(Eigen3_INCLUDE_DIRS ${EIGEN_INCLUDE})
    endif()
endif()

# Set the Spectra include directory.
if(NOT DEFINED SPECTRA_INCLUDE)
    set(SPECTRA_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/spectra/include")
endif()

# Find OpenMP.
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# --- MPI Configuration ---
find_package(MPI QUIET)
if(MPI_CXX_FOUND)
    message(STATUS "MPI found: ${MPI_CXX_INCLUDE_DIRS}")
    include_directories(${MPI_CXX_INCLUDE_DIRS})
endif()

# Include directories for project headers, Eigen3, and Spectra.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${Eigen3_INCLUDE_DIRS})
include_directories(${SPECTRA_INCLUDE})

# List the common source files.
set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/DynamicBitSet.cpp
    ${CMAKE_SOURCE_DIR}/src/GeneticAlgorithm.cpp
    ${CMAKE_SOURCE_DIR}/src/FitnessEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/GeneticOperators.cpp
    ${CMAKE_SOURCE_DIR}/src/GraphGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper.cpp
    ${CMAKE_SOURCE_DIR}/src/Grow.cpp
)

# Build the sequential executable (using main.cpp).
add_executable(GraphEVO ${COMMON_SOURCES} ${CMAKE_SOURCE_DIR}/src/main.cpp)
if(OpenMP_CXX_FOUND)
    target_link_libraries(GraphEVO OpenMP::OpenMP_CXX)
endif()

# If MPI is found, also build the MPI (island model) executable.
if(MPI_CXX_FOUND)
    add_executable(GraphEVO_IM ${COMMON_SOURCES} ${CMAKE_SOURCE_DIR}/src/mainIM.cpp)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(GraphEVO_IM OpenMP::OpenMP_CXX)
    endif()
    target_link_libraries(GraphEVO_IM MPI::MPI_CXX)
endif()