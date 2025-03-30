#include "GeneticOperators.hpp"
#include "GraphGenerator.hpp"
#include "FitnessEvaluator.hpp"
#include <random>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

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
                       int n, int degree, int symmetry, mt19937 &rng) {
    // Randomly decide for each vertex whether to copy parent's edge set.
    vector<bool> useParent1(n, false);
    uniform_real_distribution<> probDist(0.0, 1.0);
    for (int i = 0; i < n; i++) {
        if (probDist(rng) < 0.5)
            useParent1[i] = true;
    }
    
    // Create a child graph by mixing edge sets.
    Graph childGraph(n);
    for (int i = 0; i < n; i++) {
        if (useParent1[i]) {
            childGraph[i] = parent1.graph[i];
        } else {
            childGraph[i] = parent2.graph[i];
        }
    }
    // Ensure symmetry: if i is connected to j, then j must be connected to i.
    for (int i = 0; i < n; i++) {
        sort(childGraph[i].begin(), childGraph[i].end());
        childGraph[i].erase(unique(childGraph[i].begin(), childGraph[i].end()), childGraph[i].end());
        for (int j : childGraph[i]) {
            if (find(childGraph[j].begin(), childGraph[j].end(), i) == childGraph[j].end()) {
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
        // Check each vertex.
        for (int i = 0; i < n; i++) {
            // If vertex i has too few edges, try to add from candidates.
            if (childGraph[i].size() < static_cast<size_t>(degree)) {
                vector<int> candidates;
                for (int j = 0; j < n; j++) {
                    if (j == i) continue;
                    if (childGraph[i].size() < static_cast<size_t>(degree) &&
                        childGraph[j].size() < static_cast<size_t>(degree) &&
                        find(childGraph[i].begin(), childGraph[i].end(), j) == childGraph[i].end()) {
                        candidates.push_back(j);
                    }
                }
                if (!candidates.empty()) {
                    uniform_int_distribution<> cdistr(0, candidates.size() - 1);
                    int j = candidates[cdistr(rng)];
                    childGraph[i].push_back(j);
                    childGraph[j].push_back(i);
                    repaired = false;
                }
            }
            // If vertex i has too many edges, remove extras.
            else if (childGraph[i].size() > static_cast<size_t>(degree)) {
                while (childGraph[i].size() > static_cast<size_t>(degree)) {
                    uniform_int_distribution<> edistr(0, childGraph[i].size() - 1);
                    int idx = edistr(rng);
                    int j = childGraph[i][idx];
                    childGraph[i].erase(childGraph[i].begin() + idx);
                    auto it = find(childGraph[j].begin(), childGraph[j].end(), i);
                    if (it != childGraph[j].end()) childGraph[j].erase(it);
                    repaired = false;
                }
            }
        }
        repairIter++;
    }
    if (repairIter >= repairLimit) {
        cerr << "Crossover repair did not converge.\n";
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

// Revised Mutation Operator: Aggressive subgraph rewiring.
Individual mutate(const Individual &parent, double mutationRate, int targetDegree, mt19937 &rng) {
    Individual child = parent;
    int n = child.graph.size();
    uniform_real_distribution<> probDist(0.0, 1.0);
    
    // With probability mutationRate, choose a random subset (e.g., 10% of vertices) and rewire them.
    if (probDist(rng) < mutationRate) {
        vector<int> subset;
        for (int i = 0; i < n; i++) {
            if (probDist(rng) < 0.1) {
                subset.push_back(i);
            }
        }
        // Remove all edges among vertices in the subset.
        for (int i : subset) {
            for (int j : child.graph[i]) {
                // Remove i from j's list if j is in the subset.
                if (find(subset.begin(), subset.end(), j) != subset.end()) {
                    auto it = find(child.graph[j].begin(), child.graph[j].end(), i);
                    if (it != child.graph[j].end()) {
                        child.graph[j].erase(it);
                    }
                }
            }
            // Remove all edges in i that connect to vertices in the subset.
            auto newEnd = remove_if(child.graph[i].begin(), child.graph[i].end(), 
                                    [&](int j){ return find(subset.begin(), subset.end(), j) != subset.end(); });
            child.graph[i].erase(newEnd, child.graph[i].end());
        }
        // Aggressively rewire the subgraph:
        const int repairLimit = 10000;
        int repairIter = 0;
        bool rewired = false;
        while (!rewired && repairIter < repairLimit) {
            rewired = true;
            for (int i : subset) {
                // While vertex i has fewer than targetDegree edges (only counting edges within the subset),
                // try to add an edge with another vertex in the subset.
                if (child.graph[i].size() < static_cast<size_t>(targetDegree)) {
                    vector<int> candidates;
                    for (int j : subset) {
                        if (j == i) continue;
                        if (child.graph[i].size() < static_cast<size_t>(targetDegree) &&
                            child.graph[j].size() < static_cast<size_t>(targetDegree) &&
                            find(child.graph[i].begin(), child.graph[i].end(), j) == child.graph[i].end()) {
                            candidates.push_back(j);
                        }
                    }
                    if (!candidates.empty()) {
                        rewired = false;
                        uniform_int_distribution<> cdistr(0, candidates.size() - 1);
                        int j = candidates[cdistr(rng)];
                        child.graph[i].push_back(j);
                        child.graph[j].push_back(i);
                    }
                }
            }
            repairIter++;
        }
        if (repairIter >= repairLimit) {
            cerr << "Mutation subgraph rewiring did not converge.\n";
        }
    }
    
    // Recompute fitness after mutation.
    child.aspl = computeASPL(child.graph);
    child.algebraicConnectivity = computeAlgebraicConnectivity(child.graph);
    double lambda = 1.0;
    child.fitness = child.aspl - lambda * child.algebraicConnectivity;
    return child;
}