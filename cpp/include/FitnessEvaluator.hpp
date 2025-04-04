#ifndef FITNESS_EVALUATOR_HPP
#define FITNESS_EVALUATOR_HPP

#include "functions.hpp"

// Computes the average shortest path length (ASPL) of a graph.
double computeASPL(const Graph &g);

// Computes the algebraic connectivity of a graph.
double computeAlgebraicConnectivity(const Graph &g);

// Computes the theoretical lower bound on ASPL for a given n and k.
double minASPL(int n, int k);

#endif // FITNESS_EVALUATOR_HPP