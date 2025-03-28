#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>

using namespace std;


typedef vector<vector<int>> Graph;

struct Individual {
    Graph graph;
    double aspl;                   // average shortest path length (primary objective)
    double algebraicConnectivity;  // secondary objective (e.g. second smallest eigenvalue)
    double fitness;                // lower is better
};

double computeASPL(const Graph &g);
double computeAlgebraicConnectivity(const Graph &g);
Graph generateBaseGraph(int baseSize, int degree);
Graph generateSymmetricGraph(int n, int degree, int symmetry);
Individual createIndividual(int n, int k, int symmetry);
Individual mutate(const Individual &parent, double mutationRate, int targetDegree, std::mt19937 &rng);
Individual crossover(const Individual &parent1, const Individual &parent2, int n, int degree, int symmetry, std::mt19937 &rng);
double minASPL(int n, int k);
void printHeader(int n, int k, int symmetry, int populationSize, int generations, double mutationRate, double tolerance, double theoreticalLowerASPL);
void outputToCSV(const Individual &ind, double theoreticalMinASPL, int symmetry);

#endif // FUNCTIONS_HPP
