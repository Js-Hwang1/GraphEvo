#include "GraphGenerator.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <random>

// Forward declaration of the deterministic circulant method.
Graph generateBaseGraphCirculant(int baseSize, int degree) {
    // Feasibility checks.
    assert(degree <= baseSize - 1 && "Degree too high for base graph size.");
    if (degree % 2 == 1) {
        assert(baseSize % 2 == 0 && "For odd degree, base graph size must be even.");
    }
    
    Graph base(baseSize);
    
    if (degree % 2 == 0) {
        int half = degree / 2;
        for (int i = 0; i < baseSize; i++) {
            for (int j = 1; j <= half; j++) {
                int neighbor = (i + j) % baseSize;
                base[i].push_back(neighbor);
                base[neighbor].push_back(i);
            }
        }
    } else {
        int half = (degree - 1) / 2;
        for (int i = 0; i < baseSize; i++) {
            for (int j = 1; j <= half; j++) {
                int neighbor = (i + j) % baseSize;
                base[i].push_back(neighbor);
                base[neighbor].push_back(i);
            }
        }
        // Add perfect matching.
        for (int i = 0; i < baseSize / 2; i++) {
            int j = i + baseSize / 2;
            base[i].push_back(j);
            base[j].push_back(i);
        }
    }
    
    // Optional: sort neighbor lists.
    for (int i = 0; i < baseSize; i++) {
        std::sort(base[i].begin(), base[i].end());
    }
    
    return base;
}

// Greedy method to generate a base graph.
// Returns a graph (which might be incomplete) after up to maxAttempts.
Graph generateBaseGraphGreedy(int baseSize, int degree) {
    Graph base(baseSize);
    
    // Initialize the graph with a simple cycle for connectivity.
    for (int i = 0; i < baseSize; i++) {
        int j = (i + 1) % baseSize;
        base[i].push_back(j);
        base[j].push_back(i);
    }
    
    // If the target degree is 2, the cycle is complete.
    if (degree == 2)
        return base;
    
    // Use a random engine to randomize the order of vertices for each attempt.
    std::random_device rd;
    std::mt19937 rng(rd());
    
    const int maxAttempts = 10000;
    int attempts = 0;
    
    while (attempts < maxAttempts) {
        bool progressMade = false;
        // Shuffle the vertex order to try a different sequence.
        std::vector<int> order(baseSize);
        for (int i = 0; i < baseSize; i++) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        
        // For each vertex in randomized order, try to add missing edges.
        for (int idx = 0; idx < baseSize; idx++) {
            int i = order[idx];
            while (static_cast<int>(base[i].size()) < degree) {
                bool added = false;
                // Try candidates in random order.
                std::vector<int> candidates;
                for (int j = 0; j < baseSize; j++) {
                    if (i == j) continue;
                    if (std::find(base[i].begin(), base[i].end(), j) != base[i].end())
                        continue;
                    if (static_cast<int>(base[j].size()) < degree)
                        candidates.push_back(j);
                }
                if (!candidates.empty()) {
                    std::shuffle(candidates.begin(), candidates.end(), rng);
                    int j = candidates.front();
                    base[i].push_back(j);
                    base[j].push_back(i);
                    added = true;
                    progressMade = true;
                }
                if (!added) break;  // Cannot add more for vertex i.
            }
        }
        if (isValidGraph(base, degree))
            break;
        if (!progressMade) {
            // If no progress was made, increase attempts and optionally reset the graph.
            attempts++;
            // Optionally, you could reinitialize the graph with a fresh cycle.
            base.clear();
            base.resize(baseSize);
            for (int i = 0; i < baseSize; i++) {
                int j = (i + 1) % baseSize;
                base[i].push_back(j);
                base[j].push_back(i);
            }
        }
        attempts++;
    }
    
    if (!isValidGraph(base, degree)) {
        std::cerr << "Greedy method failed after " << maxAttempts 
                  << " attempts. Falling back to deterministic construction." << std::endl;
    }
    return base;
}

// Hybrid method: Try the greedy method first, then fallback to deterministic if necessary.
Graph generateBaseGraphHybrid(int baseSize, int degree) {
    Graph base = generateBaseGraphGreedy(baseSize, degree);
    if (!isValidGraph(base, degree)) {
        base = generateBaseGraphCirculant(baseSize, degree);
    }
    return base;
}

// Generates a symmetric graph with n vertices, regular degree, and symmetry.
// For symmetry > 1, each base block is generated as (degree - 1)-regular via the hybrid method,
// then an inter-block edge is added for each vertex.
Graph generateSymmetricGraph(int n, int degree, int symmetry) {
    assert(n % symmetry == 0 && "n must be divisible by symmetry");
    int baseSize = n / symmetry;
    
    // For symmetry > 1, build each base block as (degree - 1)-regular.
    int baseDegree = (symmetry > 1) ? (degree - 1) : degree;
    Graph base = generateBaseGraphHybrid(baseSize, baseDegree);
    
    Graph g(n);
    // Copy the base graph into each symmetry block.
    for (int s = 0; s < symmetry; s++) {
        for (int i = 0; i < baseSize; i++) {
            int global_i = s * baseSize + i;
            g[global_i] = base[i];  // Copy base block.
            // Adjust indices: add offset s * baseSize to each neighbor.
            for (int &neighbor : g[global_i]) {
                neighbor += s * baseSize;
            }
        }
    }
    
    // Add symmetric inter-block edges to bring each vertex to final degree.
    if (symmetry > 1) {
        for (int s = 0; s < symmetry; s++) {
            int next = (s + 1) % symmetry;
            for (int i = 0; i < baseSize; i++) {
                int u = s * baseSize + i;
                int v = next * baseSize + i;
                if (std::find(g[u].begin(), g[u].end(), v) == g[u].end()) {
                    g[u].push_back(v);
                    g[v].push_back(u);
                }
            }
        }
    }
    
    // Final validation: each vertex should have exactly 'degree' neighbors.
    for (int i = 0; i < n; i++) {
        if (static_cast<int>(g[i].size()) != degree) {
            std::cerr << "Vertex " << i << " has degree " << g[i].size() 
                      << " (expected " << degree << ")\n";
            assert(static_cast<int>(g[i].size()) == degree && "generateSymmetricGraph: vertex does not have the required degree");
        }
    }
    
    // Optional: sort neighbor lists.
    for (int i = 0; i < n; i++) {
        std::sort(g[i].begin(), g[i].end());
    }
    return g;
}