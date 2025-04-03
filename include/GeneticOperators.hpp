#ifndef GENETIC_OPERATORS_HPP
#define GENETIC_OPERATORS_HPP

#include "functions.hpp"
#include <random>

// Creates an individual by generating a symmetric graph.
Individual createIndividual(int n, int k, int symmetry, double alpha, double beta);

// Performs mutation on an individual.
Individual mutate(const Individual &parent, double mutationRate, int targetDegree, double alpha, double beta, std::mt19937 &rng);

// Performs crossover between two individuals.
Individual crossover(const Individual &parent1, const Individual &parent2, int n, int degree, int symmetry, double alpha, double beta, std::mt19937 &rng);

#endif // GENETIC_OPERATORS_HPP