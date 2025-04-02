#ifndef GENETIC_ALGORITHM_HPP
#define GENETIC_ALGORITHM_HPP

#include "functions.hpp"
#include <random>
#include <vector>

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

    // Adaptive mutation parameters.
    double bestFitness;
    int stagnationCount;
    const double minMutationRate;
    const double maxMutationRate;
    int stagnationThreshold;

    // Population and random generator.
    std::vector<Individual> population;
    std::mt19937 rng;

    // Constructor.
    GeneticAlgorithm(int n, int k, int symmetry, int populationSize, int generations,
                     double mutationRate, double tolerance);

    // Initialize the population.
    void initializePopulation();

    // Run the main GA loop.
    // Returns true if an acceptable graph is found.
    bool run();

    // Retrieve the best individual.
    Individual getBestIndividual();

};

#endif // GENETIC_ALGORITHM_HPP