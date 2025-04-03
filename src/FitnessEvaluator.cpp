#include "functions.hpp"
#include "DynamicBitSet.cpp"  
#include <Eigen/Sparse>
#include <Spectra/SymEigsSolver.h>
#include <Spectra/MatOp/SparseSymMatProd.h>
#include <Spectra/Util/SelectionRule.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <iostream>
#include <cmath>

using namespace Eigen;
using namespace std;

double computeASPL(const Graph &g) {
    int n = g.size();
    if (n == 0)
        return 0.0;
    
    // Precompute neighbor bitsets.
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
    
    // Parallelize the outer loop over source vertices.
    #ifdef _OPENMP
    #pragma omp parallel for reduction(+:totalDistance, count) schedule(dynamic)
    #endif
    for (int src = 0; src < n; src++) {
        DynamicBitset visited(n);
        visited.reset();
        visited.set(src);
        DynamicBitset current(n);
        current.reset();
        current.set(src);
        int d = 0;
        
        long long localTotal = 0;
        long long localCount = 0;
        
        while (true) {
            DynamicBitset next(n);
            next.reset();
            for (int v = 0; v < n; v++) {
                if (current.test(v)) {
                    next |= neighbor[v];
                }
            }
            // Exclude vertices already visited.
            DynamicBitset notVisited = ~visited;
            next &= notVisited;
            if (next.none())
                break;
            d++;
            for (int v = 0; v < n; v++) {
                if (next.test(v)) {
                    localTotal += d;
                    localCount++;
                }
            }
            visited |= next;
            current = next;
        }
        totalDistance += localTotal;
        count += localCount;
    }
    
    return (count > 0) ? static_cast<double>(totalDistance) / count : 0.0;
}

double computeAlgebraicConnectivity(const Graph &g) {
    int n = g.size();
    if (n == 0) return 0.0;
    
    typedef Eigen::SparseMatrix<double> SpMat;
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletList;
    tripletList.reserve(n * 4);
    
    for (int i = 0; i < n; i++) {
        int deg = g[i].size();
        tripletList.push_back(T(i, i, deg));
        for (int j : g[i]) {
            tripletList.push_back(T(i, j, -1.0));
        }
    }
    
    SpMat L(n, n);
    L.setFromTriplets(tripletList.begin(), tripletList.end());
    
    // Form M = -L.
    SpMat M = -L;
    
    Spectra::SparseSymMatProd<double> op(M);
    
    int nev = 2;                   
    int ncv = std::min(n, 6);
    
    // Instantiate solver using the operator type as the single template parameter.
    Spectra::SymEigsSolver<Spectra::SparseSymMatProd<double>> eigs(op, nev, ncv);
    eigs.init();
    
    // Compute eigenvalues using the sort rule for largest algebraic eigenvalues.
    int nconv = eigs.compute(Spectra::SortRule::LargestAlge, 1000, 1e-10);
    
    if (eigs.info() == Spectra::CompInfo::Successful && nconv >= nev) {
        Eigen::VectorXd eigenvalues = eigs.eigenvalues();
        double lambda2 = -eigenvalues(1); // Second largest eigenvalue of M gives algebraic connectivity.
        return lambda2;
    } else {
        std::cerr << "Spectra did not converge or returned fewer than 2 eigenvalues." << std::endl;
        return 0.0;
    }
}

double minASPL(int n, int k) {
    double totalEdges = static_cast<double>(k) * n / 2.0;
    double totalPairs = static_cast<double>(n) * (n - 1) / 2.0;
    double totsum = 0.0;
    vector<double> Ni;
    int i = 1;
    while (true) {
        double term = pow((k - 1), i - 1) * totalEdges;
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
    for (int idx = 0; idx < static_cast<int>(Ni.size()); idx++) {
        weighted_sum += (idx + 1) * Ni[idx];
    }
    return weighted_sum / totalPairs;
}