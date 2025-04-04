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

#include "FitnessEvaluator.hpp"
#include "GeneticOperators.hpp"
#include "Helper.hpp"
#include "Grow.hpp"
#include "GeneticAlgorithm.hpp"
#endif // FUNCTIONS_HPP
