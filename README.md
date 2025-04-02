# GraphEvo

GraphEvo is an open-source C++ library that employs Genetic Algorithms to evolve optimal graph structures. The library focuses on generating regular graphs with desirable properties, particularly optimizing for average shortest path length (ASPL) while maintaining graph connectivity and symmetry.

## Features

- Genetic Algorithm-based graph optimization
- Support for regular graphs with configurable vertex count and degree
- Optimization of average shortest path length (ASPL)
- Algebraic connectivity computation
- Adaptive mutation rates for improved convergence
- Theoretical lower bound calculations for ASPL
- Symmetry-aware graph generation

## Prerequisites

- C++ compiler with C++11 support or later
- CMake build system (version 3.10 or later)

## Building the Project

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

### Command Line Interface

The program can be run from the command line with the following parameters:

```bash
./GraphEvo -n <vertices> -k <degree> -s <symmetry> -g <generations> -p <population>
```

Parameters:
- `-n`: Number of vertices in the graph
- `-k`: Regular graph degree (number of edges per vertex)
- `-s`: Symmetry parameter
- `-g`: Number of generations to run
- `-p`: Population size for the genetic algorithm

Example:
```bash
./GraphEvo -n 32 -k 3 -s 1 -g 10000 -p 1000
```

### Library Usage

If you want to use GraphEvo as a library in your own project:

```cpp
#include "GeneticAlgorithm.hpp"

int main() {
    // Parameters for graph generation
    int n = 100;              // Number of vertices
    int k = 4;                // Regular graph degree
    int symmetry = 2;         // Symmetry parameter
    int populationSize = 100; // GA population size
    int generations = 1000;   // Number of generations
    double mutationRate = 0.1; // Initial mutation rate
    double tolerance = 0.01;   // Convergence tolerance

    // Create and run the genetic algorithm
    GeneticAlgorithm ga(n, k, symmetry, populationSize, generations, 
                       mutationRate, tolerance);
    
    if (ga.run()) {
        // Get the best graph found
        Individual bestGraph = ga.getBestIndividual();
        // Use the optimized graph...
    }

    return 0;
}
```

## Key Implementation Details

### Genetic Algorithm Components

1. **Population Management**
   - Maintains a population of graph individuals
   - Supports adaptive mutation rates based on convergence
   - Implements stagnation detection and handling

2. **Fitness Evaluation**
   - Computes average shortest path length (ASPL)
   - Evaluates algebraic connectivity
   - Compares against theoretical lower bounds

3. **Genetic Operators**
   - Crossover operations for graph recombination
   - Mutation operators for graph structure modification
   - Symmetry-preserving operations

### Optimization Strategy

The algorithm optimizes graphs by:
1. Minimizing the average shortest path length
2. Maintaining regular graph properties
3. Preserving graph connectivity
4. Respecting symmetry constraints


## License

This project is licensed under the BSD 3-Clause License - see the LICENSE file for details.

