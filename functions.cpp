#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <Eigen/Sparse>
#include <Spectra/SymEigsSolver.h>
#include <Spectra/MatOp/SparseSymMatProd.h>
#include <Spectra/Util/SelectionRule.h>
#include "functions.hpp"
#include "DynamicBitSet.cpp"

using namespace Eigen;
using namespace std;

double computeASPL(const Graph &g) {
    int n = g.size();
    if (n == 0)
        return 0.0;
    

    std::vector<DynamicBitset> neighbor;
    neighbor.reserve(n);
    for (int i = 0; i < n; i++) {
        DynamicBitset db(n);
        for (int j : g[i]) {
            db.set(j);
        }
        neighbor.push_back(db);
    }
    
    long long totalDistance = 0;
    long long count = 0;
    
    for (int src = 0; src < n; src++) {
        DynamicBitset visited(n);
        visited.reset();
        visited.set(src);
        DynamicBitset current(n);
        current.reset();
        current.set(src);
        int d = 0;
        
        while (true) {
            DynamicBitset next(n);
            next.reset();
            for (int v = 0; v < n; v++) {
                if (current.test(v)) {
                    next |= neighbor[v];
                }
            }
            DynamicBitset notVisited = ~visited;
            next &= notVisited;
            if (next.none())
                break;
            d++;

            for (int v = 0; v < n; v++) {
                if (next.test(v)) {
                    totalDistance += d;
                    count++;
                }
            }
            visited |= next;
            current = next;
        }
    }
    return (count > 0) ? static_cast<double>(totalDistance) / count : 0.0;
}


double computeAlgebraicConnectivity(const Graph &g) {
    int n = g.size();
    if (n == 0) return 0.0;
    
    // Build the sparse Laplacian matrix L = D - A.
    typedef Eigen::SparseMatrix<double> SpMat;
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletList;
    // Reserve an estimate for a k-regular graph (roughly 4 nonzeros per row)
    tripletList.reserve(n * 4);
    
    for (int i = 0; i < n; i++) {
        int deg = g[i].size();
        // Diagonal entry: L(i, i) = degree of vertex i.
        tripletList.push_back(T(i, i, deg));
        // Off-diagonals: for each edge (i, j), L(i, j) = -1.
        for (int j : g[i]) {
            tripletList.push_back(T(i, j, -1.0));
        }
    }
    
    SpMat L(n, n);
    L.setFromTriplets(tripletList.begin(), tripletList.end());
    
    SpMat M = -L;

    Spectra::SparseSymMatProd<double> op(M);
    
    int nev = 2;                   // We want the two largest eigenvalues of M.
    int ncv = std::min(n, 6);        // Number of Lanczos vectors; must be > nev.
    
    Spectra::SymEigsSolver<Spectra::SparseSymMatProd<double>> eigs(op, nev, ncv);
    eigs.init();
    

    int nconv = eigs.compute(Spectra::SortRule::LargestAlge, 1000, 1e-10);
    
    if (eigs.info() == Spectra::CompInfo::Successful && nconv >= nev) {
        Eigen::VectorXd eigenvalues = eigs.eigenvalues();
        double lambda2 = -eigenvalues(1);
        return lambda2;
    } else {
        std::cerr << "Spectra did not converge or returned fewer than 2 eigenvalues." << std::endl;
        return 0.0;
    }
}

Graph generateBaseGraph(int baseSize, int degree) {
    assert(degree <= baseSize - 1 && "Degree too high for base graph size.");
    if (degree % 2 == 1) {
        assert(baseSize % 2 == 0 && "For odd degree, base graph size must be even.");
    }

    Graph base(baseSize);
    for (int i = 0; i < baseSize; i++) {
        int j = (i + 1) % baseSize;
        base[i].push_back(j);
        base[j].push_back(i);
    }
    if (degree == 2) return base;

    int attempts = 0;
    const int maxAttempts = 10000;
    while (true) {
        bool complete = true;
        for (int i = 0; i < baseSize; i++) {
            if ((int)base[i].size() < degree) {
                complete = false;
                bool added = false;
                for (int j = 0; j < baseSize; j++) {
                    if (i == j) continue;
                    if (find(base[i].begin(), base[i].end(), j) != base[i].end())
                        continue;
                    if ((int)base[j].size() < degree) {
                        base[i].push_back(j);
                        base[j].push_back(i);
                        added = true;
                        break;
                    }
                }
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
    
    for (int i = 0; i < baseSize; i++) {
        assert((int)base[i].size() == degree && "generateBaseGraph: vertex does not have required degree");
    }
    return base;
}


Graph generateSymmetricGraph(int n, int degree, int symmetry) {
    assert(n % symmetry == 0 && "n must be divisible by symmetry");
    int baseSize = n / symmetry;
    int baseDegree = (symmetry > 1) ? (degree - 1) : degree;
    Graph base = generateBaseGraph(baseSize, baseDegree);
    
    Graph g(n);

    for (int s = 0; s < symmetry; s++) {
        for (int i = 0; i < baseSize; i++) {
            int global_i = s * baseSize + i;
            g[global_i] = base[i];  
            for (int &neighbor : g[global_i]) {
                neighbor += s * baseSize;
            }
        }
    }
    
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
    for (int i = 0; i < n; i++) {
        assert((int)g[i].size() == degree && "generateSymmetricGraph: vertex does not have the required degree");
    }
    
    return g;
}

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

double minASPL(int n, int k) {
    // Calculate total number of edges.
    double totalEdges = (double)k * n / 2.0;
    double totalPairs = (double)n * (n - 1) / 2.0;
    double totsum = 0.0;
    vector<double> Ni;
    int i = 1;
    while (true) {
        double term = pow((k-1), i - 1) * totalEdges;
        if (totsum + term >= totalPairs) {
            Ni.push_back(totalPairs - totsum);
            break;
        } else {
            Ni.push_back(term);
            totsum += term;
            i++;
        }
    }
    double weighted_sum = 0.0;
    for (int idx = 0; idx < (int)Ni.size(); idx++) {
        weighted_sum += (idx+1) * Ni[idx];
    }
    return weighted_sum / totalPairs;
}

void printHeader(int n, int k, int symmetry, int populationSize, int generations, double mutationRate, double tolerance, double theoreticalLowerASPL){
    cout << "GA parameters:\n"
    << "  n = " << n << "\n"
    << "  k = " << k << "\n"
    << "  symmetry = " << symmetry << "\n"
    << "  populationSize = " << populationSize << "\n"
    << "  generations = " << generations << "\n"
    << "  mutationRate = " << mutationRate << "\n"
    << "  tolerance = " << tolerance << "\n"
    << "  theoreticalLowerASPL = " << theoreticalLowerASPL << "\n";
}


void outputToCSV(const Individual &ind, double theoreticalMinASPL, int symmetry) {
    ofstream outFile("output.csv");
    if (!outFile.is_open()) {
        cerr << "Error: could not open output.csv for writing." << endl;
        return;
    }
    
    outFile << "Theoretical lower bound:" << fixed << setprecision(20) << theoreticalMinASPL <<endl;
    outFile << "minASPL: " << fixed << setprecision(20) << ind.aspl << endl;
    outFile << "symmetry(g): " << symmetry  << endl;
    outFile << "Algebraic Connectivity: " << fixed << setprecision(6) << ind.algebraicConnectivity << endl;
    outFile << "Adjacency list:" << endl;
    
    int n = ind.graph.size();
    for (int i = 0; i < n; i++) {
        outFile << (i + 1) << ":";
        for (int neighbor : ind.graph[i]) {
            outFile << " " << (neighbor + 1);
        }
        outFile << endl;
    }
    outFile.close();
}


