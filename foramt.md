```
GraphEVO/
├── src/
│   ├── main.cpp             // Main sequential GA driver.
│   ├── GraphGenerator.cpp   // Implementation of graph generation functions.
│   ├── FitnessEvaluator.cpp // Implementation of fitness evaluation functions.
│   ├── GeneticOperators.cpp // Implementation of crossover, mutation, etc.
│   ├── GeneticAlgorithm.cpp // Implementation of the overall GA engine.
│   ├── DynamicBitSetcpp     // Implementation of DynamicBitSet.
│   └── Helper.cpp           // Implementation of helper functions (printHeader, outputToCSV)
│
├── include/
│   ├── functions.hpp        // General function declarations and umbrella header.
│   ├── GraphGenerator.hpp   // Declarations for graph generation functions.
│   ├── FitnessEvaluator.hpp // Declarations for fitness evaluation functions.
│   ├── GeneticOperators.hpp // Declarations for genetic operators.
│   ├── GeneticAlgorithm.hpp // Declarations for the overall GA engine.
│   └── Helper.hpp           // Declarations for helper functions (printHeader, outputToCSV)
│
├── test/
│   ├── test_graph_generation.cpp      // Unit tests for graph generation.
│   ├── test_fitness_evaluator.cpp     // Unit tests for fitness evaluation.
│   ├── test_genetic_operators.cpp     // Unit tests for crossover and mutation.
│   └── CMakeLists.txt or Makefile for tests.
│
├── third_party/
│   └── spectra/             // Spectra submodule.
│
├── CMakeLists.txt           // Build configuration 
└── Makefile                 // Alternative Makefile.
```
