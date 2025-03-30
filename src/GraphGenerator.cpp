#include "GraphGenerator.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>

// Generates a base graph of size 'baseSize' that is regular with the given 'degree'
Graph generateBaseGraph(int baseSize, int degree) {
    // Ensure feasibility:
    // A vertex's degree must be at most baseSize - 1.
    assert(degree <= baseSize - 1 && "Degree too high for base graph size.");
    // For odd degree, a regular graph exists only if baseSize is even.
    if (degree % 2 == 1) {
        assert(baseSize % 2 == 0 && "For odd degree, base graph size must be even.");
    }

    // Initialize the graph with a simple cycle to ensure connectivity.
    Graph base(baseSize);
    for (int i = 0; i < baseSize; i++) {
        int j = (i + 1) % baseSize;
        base[i].push_back(j);
        base[j].push_back(i);
    }
    
    // If the target degree is exactly 2, the cycle is already complete.
    if (degree == 2) return base;

    // Greedy approach: Add edges until every vertex reaches the target degree.
    int attempts = 0;
    const int maxAttempts = 10000;
    while (true) {
        bool complete = true;
        for (int i = 0; i < baseSize; i++) {
            if (static_cast<int>(base[i].size()) < degree) {
                complete = false;
                bool added = false;
                // Try to find a candidate vertex j not adjacent to i and with room.
                for (int j = 0; j < baseSize; j++) {
                    if (i == j)
                        continue;
                    if (std::find(base[i].begin(), base[i].end(), j) != base[i].end())
                        continue;  // already connected
                    if (static_cast<int>(base[j].size()) < degree) {
                        base[i].push_back(j);
                        base[j].push_back(i);
                        added = true;
                        break;
                    }
                }
                // If no candidate was found for vertex i, break out early.
                if (!added)
                    break;
            }
        }
        if (complete)
            break;
        attempts++;
        if (attempts >= maxAttempts) {
            std::cerr << "Failed to generate a complete base graph after " 
                      << maxAttempts << " attempts." << std::endl;
            break;
        }
    }
    
    // Final check: every vertex must have exactly 'degree' neighbors.
    for (int i = 0; i < baseSize; i++) {
        assert(static_cast<int>(base[i].size()) == degree && "generateBaseGraph: vertex does not have required degree");
    }
    return base;
}

// Generates a symmetric graph with n vertices, degree 'degree', and symmetry 'symmetry'.
// For symmetry > 1, each base block is generated as (degree - 1)-regular, then an inter-block edge is added.
Graph generateSymmetricGraph(int n, int degree, int symmetry) {
    assert(n % symmetry == 0 && "n must be divisible by symmetry");
    int baseSize = n / symmetry;
    
    // For a symmetric graph to be k-regular, if there are multiple blocks, build each block as (degree - 1)-regular.
    int baseDegree = (symmetry > 1) ? (degree - 1) : degree;
    Graph base = generateBaseGraph(baseSize, baseDegree);
    
    Graph g(n);
    // Copy the base graph into each symmetry block.
    for (int s = 0; s < symmetry; s++) {
        for (int i = 0; i < baseSize; i++) {
            int global_i = s * baseSize + i;
            // Copy base[i] to g[global_i].
            g[global_i] = base[i];
            // Adjust indices: add offset s*baseSize to each neighbor.
            for (int &neighbor : g[global_i]) {
                neighbor += s * baseSize;
            }
        }
    }
    
    // Add symmetric inter-block edges to bring each vertex to the final degree.
    if (symmetry > 1) {
        for (int s = 0; s < symmetry; s++) {
            int next = (s + 1) % symmetry;
            for (int i = 0; i < baseSize; i++) {
                int u = s * baseSize + i;
                int v = next * baseSize + i;
                // Avoid duplicate edges.
                if (std::find(g[u].begin(), g[u].end(), v) == g[u].end()) {
                    g[u].push_back(v);
                    g[v].push_back(u);
                }
            }
        }
    }
    
    // Optional final validation: ensure every vertex has exactly 'degree' neighbors.
    for (int i = 0; i < n; i++) {
        assert(static_cast<int>(g[i].size()) == degree && "generateSymmetricGraph: vertex does not have the required degree");
    }
    
    return g;
}