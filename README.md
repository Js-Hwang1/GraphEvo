# GraphEvo: Genetic Algorithm for Graph Optimization

GraphEvo is a powerful C++ library with Python bindings that implements genetic algorithms for graph optimization problems. It provides efficient tools for evolving and optimizing graph structures using genetic algorithms.

## Features

- **Genetic Algorithm Implementation**: Core genetic algorithm operations including selection, crossover, and mutation
- **Graph Generation**: Tools for generating and evolving graph structures
- **Fitness Evaluation**: Customizable fitness functions for graph optimization
- **High Performance**: C++ implementation with Python bindings for optimal performance

## Installation

### From PyPI

```bash
pip install graphevo
```

### From Source

```bash
# Clone the repository
git clone https://github.com/Js-Hwang1/GraphEvo.git
cd GraphEvo/python

# Install in development mode
pip install -e .
```

## Requirements

- Python 3.7 or higher
- C++17 compatible compiler
- CMake 3.10 or higher
- Eigen3
- pybind11

## Quick Start

```python
import graphevo as ge
import networkx as nx

# Create a graph generator
generator = ge.GraphGenerator()

# Generate an initial population
population = generator.generate_population(
    population_size=100,
    num_nodes=50,
    edge_probability=0.3
)

# Create a genetic algorithm instance
ga = ge.GeneticAlgorithm(
    population=population,
    mutation_rate=0.1,
    crossover_rate=0.8,
    elite_size=5
)

# Run the genetic algorithm
best_graph = ga.evolve(
    generations=100,
    fitness_function=lambda g: nx.density(g)  # Example fitness function
)

# Access the best graph
print(f"Best graph density: {nx.density(best_graph)}")
```

## Core Components

### GraphGenerator
- Generates random graphs
- Creates initial populations
- Supports various graph generation strategies

### GeneticAlgorithm
- Implements the core genetic algorithm
- Handles selection, crossover, and mutation
- Supports customizable fitness functions

### GeneticOperators
- Provides mutation and crossover operations
- Customizable for different graph types
- Optimized for performance

### FitnessEvaluator
- Evaluates graph fitness
- Supports custom fitness functions
- Handles multi-objective optimization

## Advanced Usage

### Custom Fitness Functions

```python
def custom_fitness(graph):
    # Calculate graph properties
    density = nx.density(graph)
    clustering = nx.average_clustering(graph)
    
    # Combine into a single fitness score
    return density * clustering

# Use in genetic algorithm
ga = ge.GeneticAlgorithm(
    population=population,
    fitness_function=custom_fitness
)
```

### Custom Genetic Operators

```python
def custom_mutation(graph):
    # Implement custom mutation logic
    return mutated_graph

def custom_crossover(parent1, parent2):
    # Implement custom crossover logic
    return child_graph

# Use custom operators
ga.set_mutation_operator(custom_mutation)
ga.set_crossover_operator(custom_crossover)
```


## Citation

If you use GraphEvo in your research, please cite:

```bibtex
@software{GraphEvo,
  author = {Junsung Hwang},
  title = {GraphEvo: Genetic Algorithm for Graph Optimization},
  year = {2025},
  publisher = {GitHub},
  url = {https://github.com/Js-Hwang1/GraphEvo}
}
```

## Contact

- Author: Junsung Hwang
- Email: hwang30916@gmail.com
- GitHub: [Js-Hwang1](https://github.com/Js-Hwang1) 