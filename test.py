import graphevo

# Create a genetic algorithm instance
ga = graphevo.GeneticAlgorithm(
    n=64,                  # Number of nodes
    k=3,                   # Degree of each node
    symmetry=1,            # Symmetry constraint (1 for symmetric graphs)
    populationSize=150,    # Population size
    generations=100,      # Number of generations
    mutationRate=0.1,      # Mutation rate
    tolerance=0.0001,      # Convergence tolerance
    alpha=1.0,             # Weight for ASPL in fitness function
    beta=1.0,              # Weight for algebraic connectivity in fitness function
    computeDiversity=True  # Whether to compute population diversity metrics
)

# Initialize the population
ga.initializePopulation()

# Run the optimization
ga.run()

# Get the best individual
best = ga.getBestIndividual()
print(f"Best ASPL: {best.aspl}")
print(f"Best Algebraic Connectivity: {best.algebraicConnectivity}")