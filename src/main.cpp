#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <cstdlib>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "functions.hpp"
#include "DynamicBitSet.cpp"

using namespace std;
using namespace std::chrono;

// Global log file stream.
std::ofstream logFile;

// Initialize the log file.
void initLog() {
    logFile.open("GraphEVO.log", ios::out | ios::app);
    if (!logFile.is_open()) {
        cerr << "Error: Unable to open log file." << endl;
    }
}

// Write a message to the log file.
void logMessage(const std::string &msg) {
    if (logFile.is_open()) {
        logFile << msg << endl;
    }
}

// Print usage message if needed.
void printUsage(const char* progName) {
    cout << "Usage: " << progName << " [options]\n"
         << "Options:\n"
         << "  -n <int>       Total number of vertices (default: 64)\n"
         << "  -k <int>       Regular graph degree (default: 3)\n"
         << "  -s <int>       Symmetry parameter (default: 1)\n"
         << "  -p <int>       Population size (default: 200)\n"
         << "  -g <int>       Number of generations (default: 10000)\n"
         << "  -m <double>    Mutation rate (default: 0.1)\n"
         << "  -t <double>    Tolerance (default: 0.0001)\n";
}

int main(int argc, char* argv[]) {
    try {
        initLog();
        logMessage("GraphEVO started.");

        // Default parameters.
        int n = 64;
        int k = 3;
        int symmetry = 1;
        int populationSize = 200;
        int generations = 10000;
        double mutationRate = 0.1;
        double tolerance = 0.0001;
        
        // Simple command-line argument parsing.
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                n = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
                k = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
                symmetry = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                populationSize = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
                generations = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
                mutationRate = atof(argv[++i]);
            } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                tolerance = atof(argv[++i]);
            } else {
                printUsage(argv[0]);
                return 1;
            }
        }
        
        const double theoreticalLowerASPL = minASPL(n, k);
        printHeader(n, k, symmetry, populationSize, generations, mutationRate, tolerance, theoreticalLowerASPL);
        logMessage("Parameters: n=" + std::to_string(n) +
                   ", k=" + std::to_string(k) +
                   ", symmetry=" + std::to_string(symmetry) +
                   ", populationSize=" + std::to_string(populationSize) +
                   ", generations=" + std::to_string(generations) +
                   ", mutationRate=" + std::to_string(mutationRate) +
                   ", tolerance=" + std::to_string(tolerance));
        
        random_device rd;
        mt19937 rng(rd());
        
        // GA Population Initialization.
        vector<Individual> population;
        for (int i = 0; i < populationSize; i++) {
            population.push_back(createIndividual(n, k, symmetry));
        }
        logMessage("Population initialized with " + std::to_string(populationSize) + " individuals.");
        
        auto startTime = high_resolution_clock::now();
        
        // Adaptive mutation parameters.
        double bestFitness = std::numeric_limits<double>::infinity();
        int stagnationCount = 0;
        const double minMutationRate = 0.01;
        const double maxMutationRate = 0.5;
        const int stagnationThreshold = generations * 0.01;  // For example, 1% of generations.
        
        // GA Main Loop.
        for (int gen = 0; gen < generations; gen++) {
            bool acceptableFound = false;
            for (const auto &ind : population) {
                if (fabs(ind.aspl - theoreticalLowerASPL) / theoreticalLowerASPL < tolerance) {
                    cout << "Found acceptable graph in generation " << gen << "\n"
                         << "  ASPL = " << ind.aspl << "\n"
                         << "  theoretical lower bound = " << theoreticalLowerASPL << "\n"
                         << "  Algebraic Connectivity = " << ind.algebraicConnectivity << "\n";
                    logMessage("Acceptable graph found in generation " + std::to_string(gen));
                    outputToCSV(ind, theoreticalLowerASPL, symmetry);
                    acceptableFound = true;
                    break;
                }
            }
            if (acceptableFound)
                break;
            
            sort(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
                return a.fitness < b.fitness;
            });
            
            // Adaptive mutation rate adjustment.
            if (population[0].fitness < bestFitness - 1e-6) {
                bestFitness = population[0].fitness;
                stagnationCount = 0;
                mutationRate = max(minMutationRate, mutationRate * 0.95);
            } else {
                stagnationCount++;
                if (stagnationCount >= stagnationThreshold) {
                    mutationRate = min(maxMutationRate, mutationRate * 1.5);
                    stagnationCount = 0;
                    cout << "Increased mutation rate to " << mutationRate << " at generation " << gen << "\n";
                    logMessage("Increased mutation rate to " + std::to_string(mutationRate) + " at generation " + std::to_string(gen));
                }
            }
            
            vector<Individual> newPopulation;
            int elitism = max(1, populationSize / 10);
            for (int i = 0; i < elitism; i++) {
                newPopulation.push_back(population[i]);
            }
            
            // Offspring generation.
            int offspringCount = populationSize - elitism;
            vector<Individual> offspring(offspringCount);
            #ifdef _OPENMP
            #pragma omp parallel for schedule(dynamic)
            #endif
            for (int i = 0; i < offspringCount; i++) {
                // Each thread gets its own RNG.
                #ifdef _OPENMP
                int thread_id = omp_get_thread_num();
                #else
                int thread_id = 0;
                #endif
                mt19937 local_rng(rng());  // Thread-local RNG seeded from the global RNG.
                
                uniform_int_distribution<> distr(0, (populationSize / 2) - 1);
                int idx1 = distr(local_rng);
                int idx2 = distr(local_rng);
                Individual parent1 = population[idx1];
                Individual parent2 = population[idx2];
                Individual child = crossover(parent1, parent2, n, k, symmetry, local_rng);
                child = mutate(child, mutationRate, k, local_rng);
                offspring[i] = child;
            }
            
            newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
            population = newPopulation;
            cout << "Generation " << gen << ": Best fitness = " << population[0].fitness 
                 << ", ASPL = " << population[0].aspl << "\n";
            logMessage("Generation " + std::to_string(gen) + ": Best fitness = " + std::to_string(population[0].fitness) +
                       ", ASPL = " + std::to_string(population[0].aspl));
        }
        
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime);
        cout << "GA completed in " << duration.count() << " ms.\n";
        logMessage("GA completed in " + std::to_string(duration.count()) + " ms.");
        
        // Report best individual.
        Individual best = *min_element(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
            return a.fitness < b.fitness;
        });
        
        cout << "Best individual:\n"
             << "  ASPL = " << best.aspl << "\n"
             << "  Algebraic Connectivity = " << best.algebraicConnectivity << "\n";
        outputToCSV(best, theoreticalLowerASPL, symmetry);
        logMessage("Best individual ASPL: " + std::to_string(best.aspl) +
                   ", Algebraic Connectivity: " + std::to_string(best.algebraicConnectivity));
        
        logFile.close();
        return 0;
    } catch (std::exception &ex) {
        cerr << "Exception: " << ex.what() << endl;
        if (logFile.is_open()) {
            logFile << "Exception: " << ex.what() << endl;
            logFile.close();
        }
        return 1;
    }
}
