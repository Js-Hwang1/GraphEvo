#include "functions.hpp"
#include <iostream>
#include <algorithm>
#include <limits>
#include <string>
#include <iomanip>
#ifdef _OPENMP
#include <omp.h>
#endif

GeneticAlgorithm::GeneticAlgorithm(int n, int k, int symmetry, int populationSize, int generations, double mutationRate, double tolerance, bool useSeedGraphs, double alpha, double beta, bool computeDiversity)
    : n(n), k(k), symmetry(symmetry), populationSize(populationSize),
      generations(generations), mutationRate(mutationRate), tolerance(tolerance),
      theoreticalLowerASPL(minASPL(n, k)),
      alpha(alpha), beta(beta),
      bestFitness(std::numeric_limits<double>::infinity()),
      stagnationCount(0),
      minMutationRate(0.001),
      maxMutationRate(0.5),
      stagnationThreshold(static_cast<int>(generations * 0.1)),
      useSeedGraphs(useSeedGraphs),
      computeDiversity(computeDiversity)
{
    std::random_device rd;
    rng.seed(rd());
    
    if (useSeedGraphs) {
        grow = std::make_unique<Grow>(n, k, symmetry, alpha, beta);
    }
}

void GeneticAlgorithm::setSeedGraphDirectory(const std::string& directory) {
    if (useSeedGraphs && grow) {
        grow->loadSeedGraphs(directory);
    }
}

void GeneticAlgorithm::initializePopulation() {
    population.clear();
    
    if (useSeedGraphs && grow && grow->hasSeedGraphs()) {
        grow->initializePopulationWithSeeds(population, populationSize);
    } else {
        for (int i = 0; i < populationSize; i++) {
            population.push_back(createIndividual(n, k, symmetry, alpha, beta));
        }
    }
}

bool GeneticAlgorithm::run() {
    bool converged = false;
    int convergenceGeneration = -1;
    int extraGenerations = static_cast<int>(generations * 0.05);

    for (int gen = 0; gen < generations; gen++) {
        // Check if any individual meets the acceptance criterion.
        for (const auto &ind : population) {
            if ((fabs(ind.aspl - theoreticalLowerASPL) / theoreticalLowerASPL < tolerance) && ((int) alpha != 0)) {
                if (!converged) {
                    converged = true;
                    convergenceGeneration = gen;
                    if((int) beta == 0){
                        beta = 1;
                    }
                    std::cout << "Convergence achieved at generation " << gen 
                              << ". Continuing for " << extraGenerations 
                              << " extra generations to explore further improvements." << std::endl;
                }
            }
        }
        
        // Sort population by fitness (lower is better).
        std::sort(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
            return a.fitness < b.fitness;
        });
        
        // Adaptive mutation rate adjustment.
        if (population[0].fitness < bestFitness - 1e-6) {
            bestFitness = population[0].fitness;
            stagnationCount = 0;
            mutationRate = std::max(minMutationRate, mutationRate * 0.95);
        } else {
            stagnationCount++;
            if (stagnationCount >= stagnationThreshold) {
                mutationRate = std::min(maxMutationRate, mutationRate * 1.5);
                stagnationCount = 0;
                std::cout << "Increased mutation rate to " << mutationRate << " at generation " << gen << "\n";
                std::string msg = std::string("Increased mutation rate to ") + std::to_string(mutationRate) + " at generation " + std::to_string(gen);
                logMessage(msg);
            }
        }
        
        // Elitism: Copy top individuals.
        std::vector<Individual> newPopulation;
        int elitism = std::max(1, populationSize / 10);
        for (int i = 0; i < elitism; i++) {
            newPopulation.push_back(population[i]);
        }
        
        int offspringCount = populationSize - elitism;
        std::vector<Individual> offspring(offspringCount);
        
        // Parallel offspring generation using OpenMP.
        #ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic)
        #endif
        for (int i = 0; i < offspringCount; i++) {
            // Create a thread-local RNG seeded from the global rng.
            std::mt19937 local_rng(rng());
            std::uniform_int_distribution<> distr(0, (populationSize / 2) - 1);
            int idx1 = distr(local_rng);
            int idx2 = distr(local_rng);
            Individual parent1 = population[idx1];
            Individual parent2 = population[idx2];
            Individual child = crossover(parent1, parent2, n, k, symmetry, alpha, beta, local_rng);
            child = mutate(child, mutationRate, k, alpha, beta, local_rng);
            offspring[i] = child;
        }
        
        newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
        population = newPopulation;
        
        // Print generation statistics
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Generation " << gen << ":"
                  << "  Best fitness = " << population[0].fitness
                  << ", ASPL = " << population[0].aspl;
        
        // Compute and print diversity metrics if enabled
        if (computeDiversity) {
            DiversityMetrics diversity = measurePopulationDiversity(population);
            std::cout << ", Fitness std dev = " << diversity.fitnessStdDev
                      << ", range = " << diversity.fitnessRange 
                      << ", Avg edge diff = " << diversity.avgEdgeDiff
                      << ", path dist diversity = " << diversity.pathDistDiversity;
            
            std::string msg = std::string("Generation ") + std::to_string(gen) + ":"
                           + "  Best fitness = " + std::to_string(population[0].fitness)
                           + ", ASPL = " + std::to_string(population[0].aspl)
                           + ", absError = " + std::to_string((population[0].aspl - theoreticalLowerASPL)*100/theoreticalLowerASPL) + "%"
                           + ", Fitness std dev = " + std::to_string(diversity.fitnessStdDev)
                           + ", range = " + std::to_string(diversity.fitnessRange)
                           + ", Avg edge diff = " + std::to_string(diversity.avgEdgeDiff)
                           + ", path dist diversity = " + std::to_string(diversity.pathDistDiversity);
            logMessage(msg);
        } else {
            double absErr = (population[0].aspl - theoreticalLowerASPL)*100/theoreticalLowerASPL;
            std::string msg = std::string("Generation ") + std::to_string(gen) + ":"
                           + "  Best fitness = " + std::to_string(population[0].fitness)
                           + ", ASPL = " + std::to_string(population[0].aspl)
                           + ", absError = " + std::to_string(absErr) + "%";
            logMessage(msg);
        }
        std::cout<<"\n";

        if (converged && (gen >= convergenceGeneration + extraGenerations)) {
            std::cout << "Extra generations complete. Acceptable graph found." << std::endl;
            logMessage("Extra generations complete. Acceptable graph found.");
            outputToCSV(population[0], theoreticalLowerASPL, symmetry);
            return true;
        }
    }
    return false;
}

Individual GeneticAlgorithm::getBestIndividual() {
    return *std::min_element(population.begin(), population.end(), [](const Individual &a, const Individual &b) {
        return a.fitness < b.fitness;
    });
}
