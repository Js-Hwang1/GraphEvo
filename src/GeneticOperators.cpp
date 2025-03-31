#include "GeneticOperators.hpp"
#include "GraphGenerator.hpp"
#include "FitnessEvaluator.hpp"
#include <random>
#include <algorithm>
#include <cassert>
#include <iostream>



Individual createIndividual(int n, int degree, int symmetry) {
    Individual ind;
    ind.graph = generateSymmetricGraph(n, degree, symmetry);
    ind.aspl = computeASPL(ind.graph);
    ind.algebraicConnectivity = computeAlgebraicConnectivity(ind.graph);
    double lambda = 1.0;
    ind.fitness = ind.aspl - lambda * ind.algebraicConnectivity;
    return ind;
}

Individual crossover(const Individual &parent1, const Individual &parent2, 
                       int n, int degree, int symmetry, std::mt19937 &rng) {
    // Randomly decide for each vertex whether to copy parent's edge set.
    std::vector<bool> useParent1(n, false);
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    for (int i = 0; i < n; i++) {
        if (probDist(rng) < 0.5)
            useParent1[i] = true;
    }
    
    // Create a child graph by mixing edge sets.
    Graph childGraph(n);
    for (int i = 0; i < n; i++) {
        if (useParent1[i])
            childGraph[i] = parent1.graph[i];
        else
            childGraph[i] = parent2.graph[i];
    }
    
    // Ensure symmetry: if vertex i is connected to vertex j, then j must be connected to i.
    for (int i = 0; i < n; i++) {
        std::sort(childGraph[i].begin(), childGraph[i].end());
        childGraph[i].erase(std::unique(childGraph[i].begin(), childGraph[i].end()), childGraph[i].end());
        for (int j : childGraph[i]) {
            if (std::find(childGraph[j].begin(), childGraph[j].end(), i) == childGraph[j].end()) {
                childGraph[j].push_back(i);
            }
        }
    }
    
    // Repair procedure to enforce k-regularity.
    const int repairLimit = 10000;
    int repairIter = 0;
    bool repaired = false;
    while (!repaired && repairIter < repairLimit) {
        repaired = true;
        for (int i = 0; i < n; i++) {
            // Add edges if vertex i has too few neighbors.
            if (childGraph[i].size() < static_cast<size_t>(degree)) {
                std::vector<int> candidates;
                for (int j = 0; j < n; j++) {
                    if (j == i)
                        continue;
                    if (childGraph[i].size() < static_cast<size_t>(degree) &&
                        childGraph[j].size() < static_cast<size_t>(degree) &&
                        std::find(childGraph[i].begin(), childGraph[i].end(), j) == childGraph[i].end()) {
                        candidates.push_back(j);
                    }
                }
                if (!candidates.empty()) {
                    std::uniform_int_distribution<> cdistr(0, candidates.size() - 1);
                    int j = candidates[cdistr(rng)];
                    childGraph[i].push_back(j);
                    childGraph[j].push_back(i);
                    repaired = false;
                }
            }
            // Remove extra edges if vertex i has too many.
            else if (childGraph[i].size() > static_cast<size_t>(degree)) {
                while (childGraph[i].size() > static_cast<size_t>(degree)) {
                    std::uniform_int_distribution<> edistr(0, childGraph[i].size() - 1);
                    int idx = edistr(rng);
                    int j = childGraph[i][idx];
                    childGraph[i].erase(childGraph[i].begin() + idx);
                    auto it = std::find(childGraph[j].begin(), childGraph[j].end(), i);
                    if (it != childGraph[j].end()) childGraph[j].erase(it);
                    repaired = false;
                }
            }
        }
        repairIter++;
    }
    
    if (!isValidGraph(childGraph, degree)) {
        std::cerr << "Crossover repair did not converge after " << repairLimit 
                  << " iterations. Falling back to deterministic construction." << std::endl;
        // Fall back: regenerate a symmetric graph deterministically.
        childGraph = generateSymmetricGraph(n, degree, symmetry);
    }
    
    // Build and return the offspring.
    Individual child;
    child.graph = childGraph;
    child.aspl = computeASPL(childGraph);
    child.algebraicConnectivity = computeAlgebraicConnectivity(childGraph);
    double lambda = 1.0;
    child.fitness = child.aspl - lambda * child.algebraicConnectivity;
    return child;
}

Individual mutate(const Individual &parent, double mutationRate, int targetDegree, std::mt19937 &rng) {
    Individual child = parent;
    int n = child.graph.size();
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    if (probDist(rng) < mutationRate) {
        std::vector<int> subset;
        for (int i = 0; i < n; i++) {
            if (probDist(rng) < 0.1) {  // Approximately 10% of vertices.
                subset.push_back(i);
            }
        }
        // Remove all edges among vertices in the subset.
        for (int i : subset) {
            for (int j : child.graph[i]) {
                if (std::find(subset.begin(), subset.end(), j) != subset.end()) {
                    auto it = std::find(child.graph[j].begin(), child.graph[j].end(), i);
                    if (it != child.graph[j].end())
                        child.graph[j].erase(it);
                }
            }
            auto newEnd = std::remove_if(child.graph[i].begin(), child.graph[i].end(), 
                                    [&](int j){ return std::find(subset.begin(), subset.end(), j) != subset.end(); });
            child.graph[i].erase(newEnd, child.graph[i].end());
        }
        // Aggressively rewire the subgraph.
        const int repairLimit = 10000;
        int repairIter = 0;
        bool rewired = false;
        while (!rewired && repairIter < repairLimit) {
            rewired = true;
            for (int i : subset) {
                if (child.graph[i].size() < static_cast<size_t>(targetDegree)) {
                    std::vector<int> candidates;
                    for (int j : subset) {
                        if (j == i) continue;
                        if (child.graph[i].size() < static_cast<size_t>(targetDegree) &&
                            child.graph[j].size() < static_cast<size_t>(targetDegree) &&
                            std::find(child.graph[i].begin(), child.graph[i].end(), j) == child.graph[i].end()) {
                            candidates.push_back(j);
                        }
                    }
                    if (!candidates.empty()) {
                        rewired = false;
                        std::uniform_int_distribution<> cdistr(0, candidates.size() - 1);
                        int j = candidates[cdistr(rng)];
                        child.graph[i].push_back(j);
                        child.graph[j].push_back(i);
                    }
                }
            }
            repairIter++;
        }
        if (repairIter >= repairLimit) {
            std::cerr << "Mutation subgraph rewiring did not converge after " << repairLimit << " iterations." << std::endl;
        }
    }
    
    if (!isValidGraph(child.graph, targetDegree)) {
        std::cerr << "Mutation: Graph is not " << targetDegree << "-regular after mutation. Falling back to deterministic construction." << std::endl;
        // Fall back: regenerate the graph deterministically.
        child.graph = generateSymmetricGraph(n, targetDegree, 1); // Assuming symmetry=1 for fallback.
    }
    
    // Recompute fitness.
    child.aspl = computeASPL(child.graph);
    child.algebraicConnectivity = computeAlgebraicConnectivity(child.graph);
    double lambda = 1.0;
    child.fitness = child.aspl - lambda * child.algebraicConnectivity;
    return child;
}