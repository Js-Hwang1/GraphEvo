#ifndef GRAPH_GENERATOR_HPP
#define GRAPH_GENERATOR_HPP

#include "functions.hpp"

Graph generateBaseGraphCirculant(int baseSize, int degree);
Graph generateBaseGraphGreedy(int baseSize, int degree);
Graph generateBaseGraphHybrid(int baseSize, int degree) ;
Graph generateSymmetricGraph(int n, int degree, int symmetry);
#endif // GRAPH_GENERATOR_HPP