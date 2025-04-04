# GraphEvo: High-Performance Genetic Algorithm Framework for Graph Optimization

GraphEvo is a sophisticated C++ library with Python bindings that implements advanced genetic algorithms for graph optimization problems. The framework is specifically designed for optimizing graph structures with a focus on high performance through parallel computing and efficient data structures.

## Technical Overview

### Core Algorithms

#### 1. Genetic Algorithm Implementation
- **Selection Method:**  
  Tournament selection with adaptive pressure based on population diversity.

- **Mutation Strategy:**  
  - **Smart Mutation Operator:**  
    Our smart mutation operator is designed to make significant yet controlled modifications to the graph structure. It employs a heuristic based on the shortest path distribution observed in graphs that approach the theoretical minimum ASPL as determined by Moore’s rule.
  - **Heuristic Basis:**  
    The operator analyzes the distribution of shortest path lengths in near-optimal (minASPL) graphs—where, according to Moore’s bound, the maximum number of vertices reachable at each distance level is capped—and uses this insight to guide the mutation. This approach ensures that the mutation respects the intrinsic structure of optimal graphs while still introducing enough diversity to escape local optima.
  - **Adaptive Mutation Rate:**  
    The mutation rate is dynamically adjusted based on the algorithm’s progress. If the population stagnates (i.e., no significant improvement in fitness over several generations), the mutation rate is increased to explore a wider search space. Conversely, when progress is made, the rate is gradually reduced to fine-tune the solution.

- **Crossover Implementation:**  
  - **Master Regulatory Gene (MRG) Based Crossover:**  
    This method preserves graph regularity and symmetry constraints, incorporating intelligent edge preservation to maintain structural integrity.

#### 2. Fitness Evaluation
- **Average Shortest Path Length (ASPL)**:
  - Parallel bitwise multi-source BFS implementation
  - OpenMP parallelization for source vertices
  - Custom DynamicBitSet class for efficient bitwise operations
  - O(V * (V + E)/w) complexity where w is the machine word size
- **Algebraic Connectivity**:
  - Efficient sparse matrix implementation using Eigen
  - Spectra library for fast eigenvalue computation
  - Optimized Laplacian matrix construction
  - Parallel computation of second smallest eigenvalue

#### 3. Graph Generation and Evolution
- **Graph Generator**:
  - Degree-constrained random graph generation
  - Symmetry-preserving operations
  - Efficient adjacency list representation
- **Population Diversity Metrics**:
  - Fitness standard deviation
  - Fitness range analysis
  - Average edge difference computation
  - Path distribution diversity measurement

### Performance Optimizations

1. **Parallel Computing**:
   - OpenMP parallelization for fitness evaluation
   - Vectorized bitwise operations
   - Efficient memory management with custom allocators

2. **Data Structures**:
   - Custom DynamicBitSet for O(1) set operations
   - Sparse matrix representations for large graphs
   - Cache-friendly data layouts

3. **Algorithm Optimizations**:
   - Early termination conditions
   - Adaptive parameter tuning
   - Efficient graph isomorphism checking

## Installation

### From PyPI

```bash
pip install graphevo
```

### From Source

```bash
# Clone the repository
git clone https://github.com/Js-Hwang1/GraphEvo.git
cd GraphEvo

# Install in development mode
pip install -e .
```

## Requirements

- Python ≥ 3.7
- C++17 compatible compiler
- CMake ≥ 3.10
- Eigen3
- OpenMP (optional, for parallel computation)
- pybind11

## Usage Examples

### Basic Usage
```python
import graphevo as ge
import networkx as nx

# Initialize with specific parameters
generator = ge.GraphGenerator(
    symmetry_preserving=True,
    degree_constrained=True
)

# Generate initial population with constraints
population = generator.generate_population(
    population_size=100,
    num_nodes=50,
    degree=3,  # k-regular graph
    symmetry_level=2
)

# Configure genetic algorithm with advanced parameters
ga = ge.GeneticAlgorithm(
    population=population,
    mutation_rate=0.1,
    crossover_rate=0.8,
    elite_size=5,
    diversity_threshold=0.3,
    adaptive_mutation=True
)

# Define a multi-objective fitness function
def custom_fitness(graph):
    aspl = ge.compute_aspl(graph)  # Efficient parallel implementation
    alg_conn = ge.compute_algebraic_connectivity(graph)
    return 0.7 * aspl + 0.3 * alg_conn

# Run evolution with custom parameters
best_graph = ga.evolve(
    generations=100,
    fitness_function=custom_fitness,
    convergence_threshold=1e-6,
    diversity_maintenance=True
)
```

### Advanced Configuration
```python
# Configure advanced genetic operators
ga.set_selection_method("tournament", tournament_size=3)
ga.set_crossover_method("mrg", preservation_rate=0.8)
ga.set_mutation_method("smart", adaptive_rate=True)

# Enable diversity maintenance
ga.enable_diversity_tracking(
    measure_interval=5,
    min_diversity=0.2,
    adjustment_rate=0.1
)
```

## Performance Benchmarks

| Graph Size | Optimization Task | Time (s) | Memory (MB) |
|------------|------------------|----------|-------------|
| 1000 nodes | ASPL            | 0.45     | 128        |
| 1000 nodes | Alg. Conn.      | 0.82     | 256        |
| 5000 nodes | ASPL            | 2.15     | 512        |
| 5000 nodes | Alg. Conn.      | 4.32     | 1024       |

## Citation

If you use GraphEvo in your research, please cite:

```bibtex
@software{GraphEvo,
  author = {Junsung Hwang},
  title = {GraphEvo: High-Performance Genetic Algorithm Framework for Graph Optimization},
  year = {2024},
  publisher = {GitHub},
  url = {https://github.com/Js-Hwang1/GraphEvo}
}
```

## Contact

- Author: Junsung Hwang
- Email: junsung.hwang@kaist.ac.kr
- GitHub: [Js-Hwang1](https://github.com/Js-Hwang1) 