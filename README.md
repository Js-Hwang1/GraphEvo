# GraphEvo

GraphEvo is an open-source C++ library that implements a Genetic Algorithm-based approach for evolving optimal graph structures. The library focuses on generating regular graphs with desirable properties, specifically optimizing both the average shortest path length (ASPL) and algebraic connectivity (AC) while maintaining graph connectivity and symmetry.

## Features

- Genetic Algorithm-based graph optimization
- Support for regular graphs with configurable vertex count, degree, and symmetry
- Dual-objective optimization: minimizing ASPL and maximizing algebraic connectivity
- Adaptive mutation rates for improved convergence
- Smart mutation strategy based on path distribution analysis
- Configurable alpha/beta parameters for adjustable fitness function
- Population diversity metrics with optional computation
- Multi-level parallelization:
  * OpenMP for fine-grained parallelization
  * MPI-based Island Model for distributed evolution

## Prerequisites

- C++ compiler with C++11 support or later
- CMake build system (version 3.10 or later)
- OpenMP support (for fine-grained parallelization)
- MPI implementation (for Island Model execution)

## Getting Started

Clone the repository recursively to include all submodules:

```bash
git clone --recursive git@github.com:Js-Hwang1/GraphEvo.git
cd GraphEvo
```

## Building the Project

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

### Command Line Interface

The program can be executed from the command line with the following parameters:

```bash
./GraphEvo -n <vertices> -k <degree> -s <symmetry> -g <generations> -p <population> [-a <alpha>] [-b <beta>] [-div <0|1>]
```

Parameters:
- `-n`: Number of vertices in the graph
- `-k`: Regular graph degree (number of edges per vertex)
- `-s`: Symmetry parameter
- `-g`: Number of generations to run
- `-p`: Population size for the genetic algorithm
- `-a`: Alpha parameter for ASPL weight in fitness function (default: 1.0)
- `-b`: Beta parameter for AC weight in fitness function (default: 1.0)
- `-div`: Enable/disable diversity metrics computation (0: disabled, 1: enabled, default: 0)

Example:
```bash
./GraphEvo -n 32 -k 3 -s 1 -g 10000 -p 1000 -a 1 -b 1 -div 1
```

### Island Model Execution

For distributed evolution using the Island Model:

```bash
mpirun -np <num_islands> ./mainIM -n <vertices> -k <degree> -s <symmetry> -g <generations> -p <population> [-mi <migration_interval>]
```

Additional Island Model parameters:
- `-np`: Number of islands (MPI processes)
- `-mi`: Migration interval between islands (default: 10 generations)

Example:
```bash
mpirun -np 4 ./mainIM -n 32 -k 3 -s 1 -g 10000 -p 1000 -mi 20
```

### Library Usage

To integrate GraphEvo into your project:

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
    double alpha = 1.0;       // Weight for ASPL in fitness
    double beta = 1.0;        // Weight for AC in fitness
    bool computeDiversity = false; // Whether to compute diversity metrics

    // Initialize and execute the genetic algorithm
    GeneticAlgorithm ga(n, k, symmetry, populationSize, generations, 
                       mutationRate, tolerance, false, alpha, beta, computeDiversity);
    
    if (ga.run()) {
        // Retrieve the optimal graph
        Individual optimalGraph = ga.getBestIndividual();
        // Process the optimized graph...
    }

    return 0;
}
```

## Implementation Details

### Genetic Algorithm Architecture

1. **Population Management**
   - Maintains a population of graph individuals
   - Implements adaptive mutation rates based on convergence
   - Features stagnation detection and handling mechanisms
   - Optional computation of population diversity metrics

2. **Fitness Evaluation**
   - Implements dual-objective optimization:
     * Minimization of average shortest path length (ASPL)
     * Maximization of algebraic connectivity (AC)
   - Configurable weights (alpha/beta) for objective balancing
   - Theoretical lower bound validation for ASPL

3. **Genetic Operators**
   - Smart mutation strategy utilizing path distribution analysis
   - MRG (Master Regulatory Gene) crossover operations for graph recombination
   - Mutation operators preserving graph structure
   - Symmetry-preserving operations

### Optimization Methodology

The algorithm employs a weighted multi-objective approach:
1. Optimizes the weighted sum of ASPL and algebraic connectivity
2. Maintains k-regular graph properties
3. Ensures graph connectivity
4. Preserves specified symmetry constraints
5. Utilizes smart mutation to enhance path distribution

### Parallelization Architecture

1. **Fine-grained Parallelization (OpenMP)**
   - Parallel offspring generation in genetic operators
   - Concurrent fitness evaluation
   - Thread-safe random number generation
   - Dynamic scheduling for load balancing

2. **Island Model (MPI)**
   - Distributed evolution across multiple processes
   - Ring topology for migration
   - Configurable migration intervals
   - Best individual exchange between islands
   - Global fitness reduction for convergence detection

### Diversity Analysis

When enabled (-div 1), the algorithm computes the following normalized metrics:
1. Fitness standard deviation
2. Fitness range
3. Average edge difference between graphs
4. Path distribution diversity

All metrics are normalized to the [0,1] range for standardized interpretation.

## License

This project is licensed under the BSD 3-Clause License - see the LICENSE file for details.

