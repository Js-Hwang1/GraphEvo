#ifndef GENETIC_ALGORITHM_HPP
#define GENETIC_ALGORITHM_HPP

#include "functions.hpp"
#include <random>
#include <vector>
#include <memory>

class GeneticAlgorithm {
public:
    // GA parameters.
    int n;                // Total vertices.
    int k;                // Regular graph degree.
    int symmetry;         // Symmetry parameter.
    int populationSize;
    int generations;
    double mutationRate;
    double tolerance;
    double theoreticalLowerASPL;
    double alpha;
    double beta;

    // Adaptive mutation parameters.
    double bestFitness;
    int stagnationCount;
    const double minMutationRate;
    const double maxMutationRate;
    int stagnationThreshold;

    // Population and random generator.
    std::vector<Individual> population;
    std::mt19937 rng;
    
    // Seed graph functionality
    std::unique_ptr<Grow> grow;
    bool useSeedGraphs;
    bool computeDiversity;  // Flag to control diversity computation

    // Constructor.
    GeneticAlgorithm(int n, int k, int symmetry, int populationSize, int generations,
                     double mutationRate, double tolerance, bool useSeedGraphs = false,
                     double alpha = 1.0, double beta = 1.0, bool computeDiversity = false);

    // Initialize the population.
    void initializePopulation();

    // Run the main GA loop.
    // Returns true if an acceptable graph is found.
    bool run();

    // Retrieve the best individual.
    Individual getBestIndividual();
    
    // Set seed graph directory
    void setSeedGraphDirectory(const std::string& directory);
};

#endif // GENETIC_ALGORITHM_HPP