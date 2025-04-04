```
GraphEVO/
├── cpp/                    # Core C++ implementation
│   ├── src/
│   │   ├── main.cpp             # Main sequential GA driver
│   │   ├── mainIM.cpp           # MPI-based Island Model implementation
│   │   ├── GraphGenerator.cpp   # Implementation of graph generation functions
│   │   ├── FitnessEvaluator.cpp # Implementation of fitness evaluation functions
│   │   ├── GeneticOperators.cpp # Implementation of crossover, mutation, diversity metrics
│   │   ├── GeneticAlgorithm.cpp # Implementation of the overall GA engine
│   │   ├── DynamicBitSet.cpp    # Implementation of DynamicBitSet
│   │   ├── Grow.cpp            # Implementation of seed graph functionality
│   │   └── Helper.cpp          # Implementation of helper functions
│   │
│   ├── include/
│   │   ├── functions.hpp        # General function declarations and umbrella header
│   │   ├── GraphGenerator.hpp   # Declarations for graph generation functions
│   │   ├── FitnessEvaluator.hpp # Declarations for fitness evaluation functions
│   │   ├── GeneticOperators.hpp # Declarations for genetic operators
│   │   ├── GeneticAlgorithm.hpp # Declarations for the overall GA engine
│   │   ├── Grow.hpp            # Declarations for seed graph functionality
│   │   └── Helper.hpp          # Declarations for helper functions
│   │
│   ├── third_party/          # External dependencies
│   │    └── eigen/            # eigen library 
│   │    └── spectra/          # Spectra library for eigenvalue computation
│   │
│   └── CMakeLists.txt          # C++ build configuration
│
├── python/                 # Python wrapper
│   ├── setup.py           # Python package setup
│   ├── pybind11/          # pybind11 dependency
│   ├── graphevo/          # Python package
│   │   ├── __init__.py    # Package initialization
│   │   └── core.py        # Python interface to C++ core
│   └── tests/             # Python wrapper tests
│       └── test_graphevo.py
│
│
│
├── README.md            # Project documentation
├── STRUCTURE.md         # This file
└── .gitignore          # Git ignore patterns
``` 