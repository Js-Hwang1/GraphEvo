#include "GeneticOperators.hpp"
#include "GraphGenerator.hpp"
#include "FitnessEvaluator.hpp"
#include <random>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>

Individual createIndividual(int n, int degree, int symmetry, double alpha, double beta) {
    Individual ind;
    ind.graph = generateSymmetricGraph(n, degree, symmetry);
    ind.aspl = computeASPL(ind.graph);
    ind.algebraicConnectivity = computeAlgebraicConnectivity(ind.graph);
    ind.fitness = alpha * ind.aspl - beta * ind.algebraicConnectivity;
    return ind;
}

//MRG
Individual crossover(const Individual &parent1, const Individual &parent2, int n, int degree, int symmetry, double alpha, double beta, std::mt19937 &rng) {
    // Use a random distribution
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    // Randomly choose a primary parent
    bool primaryIsParent1 = (probDist(rng) < 0.5);
    const Individual &primary = primaryIsParent1 ? parent1 : parent2;
    const Individual &secondary = primaryIsParent1 ? parent2 : parent1;
    
    // Initialize an empty child graph with n vertices
    Graph childGraph(n);
    std::vector<bool> isSet(n, false);
    
    // Step 1: Select master nodes (e.g., 10% of vertices)
    double masterFraction = 0.1;
    std::vector<int> masterNodes;
    for (int i = 0; i < n; i++) {
        if (probDist(rng) < masterFraction) {
            masterNodes.push_back(i);
        }
    }
    
    // Step 2: Copy master nodes from the primary parent's graph
    for (int m : masterNodes) {
        childGraph[m] = primary.graph[m];
        isSet[m] = true;
        // Enforce symmetry for master node edges
        for (int j : childGraph[m]) {
            if (j >= 0 && j < n) {
                if (std::find(childGraph[j].begin(), childGraph[j].end(), m) == childGraph[j].end()) {
                    childGraph[j].push_back(m);
                }
            }
        }
    }
    
    // Step 3: Propagate master node influence to their neighbors
    for (int m : masterNodes) {
        for (int neigh : primary.graph[m]) {
            if (neigh >= 0 && neigh < n && !isSet[neigh]) {
                childGraph[neigh] = primary.graph[neigh];
                isSet[neigh] = true;
                // Enforce symmetry for the neighbor
                for (int k : childGraph[neigh]) {
                    if (k >= 0 && k < n) {
                        if (std::find(childGraph[k].begin(), childGraph[k].end(), neigh) == childGraph[k].end()) {
                            childGraph[k].push_back(neigh);
                        }
                    }
                }
            }
        }
    }
    
    // Step 4: For remaining vertices not set, choose randomly between primary and secondary parent's graph
    for (int i = 0; i < n; i++) {
        if (!isSet[i]) {
            if (probDist(rng) < 0.5)
                childGraph[i] = primary.graph[i];
            else
                childGraph[i] = secondary.graph[i];
            isSet[i] = true;
            // Enforce symmetry
            for (int j : childGraph[i]) {
                if (j >= 0 && j < n) {
                    if (std::find(childGraph[j].begin(), childGraph[j].end(), i) == childGraph[j].end()) {
                        childGraph[j].push_back(i);
                    }
                }
            }
        }
    }
    
    // Step 5: Enforce global symmetry as a safeguard
    for (int i = 0; i < n; i++) {
        for (int j : childGraph[i]) {
            if (j >= 0 && j < n) {
                if (std::find(childGraph[j].begin(), childGraph[j].end(), i) == childGraph[j].end()) {
                    childGraph[j].push_back(i);
                }
            }
        }
    }
    
    // Step 6: Deterministic repair procedure to enforce k-regularity
    // Remove extra edges
    for (int i = 0; i < n; i++) {
        if (childGraph[i].size() > static_cast<size_t>(degree)) {
            std::sort(childGraph[i].begin(), childGraph[i].end());
            while (childGraph[i].size() > static_cast<size_t>(degree)) {
                int j = childGraph[i].back();
                childGraph[i].pop_back();
                auto it = std::find(childGraph[j].begin(), childGraph[j].end(), i);
                if (it != childGraph[j].end()) {
                    childGraph[j].erase(it);
                }
            }
        }
    }
    
    // Add missing edges using a matching algorithm after pre-checking parity conditions
    std::vector<int> diff(n, 0);
    for (int i = 0; i < n; i++) {
        diff[i] = degree - childGraph[i].size();
    }
    int total_deficit = 0;
    for (int i = 0; i < n; i++) {
        total_deficit += diff[i];
    }
    // Instead of a direct fallback, we now attempt multiple randomized matchings.
    bool foundMatching = false;
    int matchingAttempts = 0;
    const int maxMatchingAttempts = 10000;
    std::vector<std::pair<int, int>> bestMatching;
    
    // Prepare a list of deficit slots (each vertex appears diff[i] times).
    std::vector<int> slots;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < diff[i]; j++) {
            slots.push_back(i);
        }
    }
    
    if(total_deficit % 2 == 0) {
        while (!foundMatching && matchingAttempts < maxMatchingAttempts) {
            // Create a shuffled copy of slots to try a different matching order.
            std::vector<int> shuffledSlots = slots;
            std::shuffle(shuffledSlots.begin(), shuffledSlots.end(), rng);
            std::vector<bool> used(shuffledSlots.size(), false);
            std::vector<std::pair<int, int>> currentMatching;
            
            // Recursive lambda to search for a perfect matching.
            std::function<bool(int)> searchMatching = [&](int index) -> bool {
                // Skip already used slots.
                while (index < shuffledSlots.size() && used[index]) {
                    index++;
                }
                if (index == shuffledSlots.size()) {
                    return true; // All slots matched.
                }
                used[index] = true;
                int u = shuffledSlots[index];
                for (int j = index + 1; j < shuffledSlots.size(); j++) {
                    if (!used[j]) {
                        int v = shuffledSlots[j];
                        if (u == v) continue; // Should never happen, but safeguard.
                        // Only add edge if not already present.
                        if (std::find(childGraph[u].begin(), childGraph[u].end(), v) != childGraph[u].end()) {
                            continue;
                        }
                        used[j] = true;
                        currentMatching.push_back({u, v});
                        if (searchMatching(index + 1))
                            return true;
                        currentMatching.pop_back();
                        used[j] = false;
                    }
                }
                used[index] = false;
                return false;
            };
            
            if (searchMatching(0)) {
                bestMatching = currentMatching;
                foundMatching = true;
            }
            matchingAttempts++;
        }
    }
    
    if (foundMatching) {
        // Add the matching edges to the graph.
        for (auto &edge : bestMatching) {
            int u = edge.first, v = edge.second;
            childGraph[u].push_back(v);
            childGraph[v].push_back(u);
        }
    }
    
    // Additional robust repair loop: try to repair any remaining deficiencies using a greedy approach
    int repairAttempts = 0;
    const int maxRepairAttempts = 10000;
    while (!isValidGraph(childGraph, degree) && repairAttempts < maxRepairAttempts) {
        for (int i = 0; i < n; i++) {
            while (childGraph[i].size() < static_cast<size_t>(degree)) {
                bool edgeAdded = false;
                for (int j = 0; j < n; j++) {
                    if (i == j) continue;
                    if (childGraph[i].size() < static_cast<size_t>(degree) &&
                        childGraph[j].size() < static_cast<size_t>(degree) &&
                        std::find(childGraph[i].begin(), childGraph[i].end(), j) == childGraph[i].end()) {
                        // Add edge between i and j.
                        childGraph[i].push_back(j);
                        childGraph[j].push_back(i);
                        edgeAdded = true;
                        break;  // Break out of inner loop to re-check conditions.
                    }
                }
                if (!edgeAdded) {
                    break;  // No edge could be added for vertex i.
                }
            }
        }
        repairAttempts++;
    }
    
    if (!isValidGraph(childGraph, degree)) {
        childGraph = generateSymmetricGraph(n, degree, symmetry);
    }

    
    // Build and return the offspring individual.
    Individual child;
    child.graph = childGraph;
    child.aspl = computeASPL(childGraph);
    child.algebraicConnectivity = computeAlgebraicConnectivity(childGraph);
    child.fitness = alpha * child.aspl - beta * child.algebraicConnectivity;
    return child;
}

Individual mutate(const Individual &parent, double mutationRate, int targetDegree, double alpha, double beta, std::mt19937 &rng) {
    // Create a copy of the parent as the initial child.
    Individual child = parent;
    int n = child.graph.size();
    std::uniform_real_distribution<> probDist(0.0, 1.0);

    // With probability mutationRate, perform an opt-2 mutation.
    if (probDist(rng) < mutationRate) {
        // Collect all unique undirected edges (each edge only once).
        std::vector<std::pair<int, int>> edges;
        for (int i = 0; i < n; i++) {
            for (int j : child.graph[i]) {
                if (j > i) {  // Ensure uniqueness.
                    edges.push_back({i, j});
                }
            }
        }
        // Proceed only if at least two edges exist.
        if (edges.size() >= 2) {
            int idx1 = -1, idx2 = -1;
            bool validPair = false;
            // Try up to 100 times to select two edges with no common vertices.
            for (int attempt = 0; attempt < 100; attempt++) {
                idx1 = std::uniform_int_distribution<>(0, edges.size()-1)(rng);
                idx2 = std::uniform_int_distribution<>(0, edges.size()-1)(rng);
                if (idx1 == idx2)
                    continue;
                auto e1 = edges[idx1];
                auto e2 = edges[idx2];
                if (e1.first != e2.first && e1.first != e2.second &&
                    e1.second != e2.first && e1.second != e2.second) {
                    validPair = true;
                    break;
                }
            }
            if (validPair) {
                auto e1 = edges[idx1];
                auto e2 = edges[idx2];
                // Two possible 2-opt swap options:
                // Option 1: (e1.first, e2.first) and (e1.second, e2.second)
                // Option 2: (e1.first, e2.second) and (e1.second, e2.first)
                bool option = (probDist(rng) < 0.5);
                std::pair<int, int> newEdge1, newEdge2;
                if (option) {
                    newEdge1 = {e1.first, e2.first};
                    newEdge2 = {e1.second, e2.second};
                } else {
                    newEdge1 = {e1.first, e2.second};
                    newEdge2 = {e1.second, e2.first};
                }
                // Lambda to remove an edge from the graph.
                auto removeEdge = [&](int u, int v) {
                    auto it = std::find(child.graph[u].begin(), child.graph[u].end(), v);
                    if (it != child.graph[u].end()) {
                        child.graph[u].erase(it);
                    }
                };
                // Lambda to add an edge (if not already present) and enforce symmetry.
                auto addEdge = [&](int u, int v) {
                    if (std::find(child.graph[u].begin(), child.graph[u].end(), v) == child.graph[u].end()) {
                        child.graph[u].push_back(v);
                    }
                };
                // Remove the original edges.
                removeEdge(e1.first, e1.second);
                removeEdge(e1.second, e1.first);
                removeEdge(e2.first, e2.second);
                removeEdge(e2.second, e2.first);
                // Add the new edges with symmetry.
                addEdge(newEdge1.first, newEdge1.second);
                addEdge(newEdge1.second, newEdge1.first);
                addEdge(newEdge2.first, newEdge2.second);
                addEdge(newEdge2.second, newEdge2.first);
            }
        }
    }

    // Validate the mutated graph; if it fails to be k-regular, fall back to a deterministic construction.
    if (!isValidGraph(child.graph, targetDegree)) {
        child.graph = generateSymmetricGraph(n, targetDegree, 1);  // Fallback with symmetry = 1.
    }

    // Update fitness metrics for the mutated individual
    child.aspl = computeASPL(child.graph);
    child.algebraicConnectivity = computeAlgebraicConnectivity(child.graph);
    child.fitness = alpha * child.aspl - beta * child.algebraicConnectivity;
    
    return child;
}