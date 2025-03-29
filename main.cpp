#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "functions.hpp"

using namespace std;
using namespace std::chrono;

int main() {
    const int n = 64;                                         // Total number of vertices (must be divisible by symmetry)
    const int k = 3;                                          // Regular graph degree
    const int symmetry = 1;                                   // Symmetry parameter (g)
    const int populationSize = 100;
    const int generations =10000;
    double mutationRate = 0.1;                                 // Recommended to set it to 0.05 - 0.1
    const double minMutationRate = 0.01;
    const double maxMutationRate = 0.5;
    const int stagnationThreshold = (int) generations*0.01;    // Increase mutation rate if no improvements.
    double bestFitness = std::numeric_limits<double>::infinity();
    int stagnationCount = 0;
    
    const double tolerance = 0.001;    // Tolerance for acceptance criterion (currently set to accept within 0.1%)
    
    const double theoreticalLowerASPL = minASPL(n, k);
    
    printHeader(n,k,symmetry,populationSize,generations,mutationRate,tolerance,theoreticalLowerASPL);

    random_device rd;
    mt19937 rng(rd());
    
    // ============ GA Population Initialization ============
    vector<Individual> population;
    for (int i = 0; i < populationSize; i++) {
        population.push_back(createIndividual(n, k, symmetry));
    }
    
    auto startTime = high_resolution_clock::now();
    
    // =================== GA Main Loop ===================
    for (int gen = 0; gen < generations; gen++) {
        bool acceptableFound = false;
        for (const auto &ind : population) {
            if (fabs(ind.aspl - theoreticalLowerASPL) / theoreticalLowerASPL < tolerance) {
                cout << "Found acceptable graph in generation " << gen << "\n"
                     << "  ASPL = " << ind.aspl << "\n"
                     << "  theoretical lower bound = " << theoreticalLowerASPL << "\n"
                     << "  Algebraic Connectivity = " << ind.algebraicConnectivity << "\n";
                acceptableFound = true;
                outputToCSV(ind, theoreticalLowerASPL, symmetry);
                break;
            }
        }
        if (acceptableFound)
            break;
        
        sort(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
            return a.fitness < b.fitness;
        });
    

        // Adaptive mutation rate changes!!!
        if (population[0].fitness < bestFitness - 1e-6) { // Use a small epsilon for improvement
            bestFitness = population[0].fitness;
            stagnationCount = 0;
            // Slowly decrease mutation rate when progress is good.
            mutationRate = max(minMutationRate, mutationRate * 0.95);
        } else {
            stagnationCount++;
            // Increase mutation rate if progress stalls.
            if (stagnationCount >= stagnationThreshold) {
                mutationRate = min(maxMutationRate, mutationRate * 1.5);
                stagnationCount = 0; // reset counter after increasing mutation rate
                cout << "Increased mutation rate to " << mutationRate << " at generation " << gen << "\n";
            }
        }


        vector<Individual> newPopulation;
        int elitism = max(1, populationSize / 10);
        for (int i = 0; i < elitism; i++) {
            newPopulation.push_back(population[i]);
        }
        
        uniform_int_distribution<> distr(0, (populationSize / 2) - 1);
        while (newPopulation.size() < static_cast<size_t>(populationSize)) {
            int idx1 = distr(rng);
            int idx2 = distr(rng);
            Individual parent1 = population[idx1];
            Individual parent2 = population[idx2];
            Individual child = crossover(parent1, parent2, n, k, symmetry, rng);
            child = mutate(child, mutationRate, k, rng);
            newPopulation.push_back(child);
        }
        
        population = newPopulation;
        cout << "Generation " << gen << ": Best fitness = " << population[0].fitness 
             << ", ASPL = " << population[0].aspl << "\n";
    }
    
    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime);
    cout << "GA completed in " << duration.count() << " ms.\n";
    
    // ============ Report Best Individual ============
    Individual best = *min_element(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
        return a.fitness < b.fitness;
    });
    
    cout << "Best individual:\n"
         << "  ASPL = " << best.aspl << "\n"
         << "  Algebraic Connectivity = " << best.algebraicConnectivity << "\n";
         outputToCSV(best, theoreticalLowerASPL, symmetry);
    return 0;
}

