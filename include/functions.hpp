#ifndef GRAPH_EVO_FUNCTIONS_HPP
#define GRAPH_EVO_FUNCTIONS_HPP

#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>

typedef std::vector<std::vector<int>> Graph;

struct Individual {
    Graph graph;
    double aspl;                  
    double algebraicConnectivity; 
    double fitness;               
};

#include "GraphGenerator.hpp"
#include "FitnessEvaluator.hpp"
#include "GeneticOperators.hpp"
#include "GeneticAlgorithm.hpp"
#include "Helper.hpp"

#endif // FUNCTIONS_HPP
