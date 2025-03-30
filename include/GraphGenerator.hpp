#ifndef GRAPH_GENERATOR_HPP
#define GRAPH_GENERATOR_HPP

#include "functions.hpp"

// Generates a base graph (of size baseSize) that is regular with the given degree.
Graph generateBaseGraph(int baseSize, int degree);

// Generates a symmetric graph with n vertices, regular degree, and a specified symmetry.
Graph generateSymmetricGraph(int n, int degree, int symmetry);

#endif // GRAPH_GENERATOR_HPP