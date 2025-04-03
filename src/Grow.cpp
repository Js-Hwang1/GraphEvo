#include "Grow.hpp"
#include "functions.hpp"
#include "Helper.hpp"
#include "FitnessEvaluator.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>

Grow::Grow(int n, int k, int symmetry, double alpha, double beta)
    : n(n), k(k), symmetry(symmetry), alpha(alpha), beta(beta) {}

void Grow::loadSeedGraphs(const std::string& seedPath) {
    seedGraphs.clear();
    
    try {
        if (std::filesystem::is_directory(seedPath)) {
            // Load all CSV files from the directory
            for (const auto& entry : std::filesystem::directory_iterator(seedPath)) {
                if (entry.path().extension() == ".csv") {
                    try {
                        Individual graph = parseGraphFromCSV(entry.path().string());
                        if (graphMatchesParameters(graph)) {
                            seedGraphs.push_back(graph);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error loading seed graph from " << entry.path() << ": " << e.what() << std::endl;
                    }
                }
            }
        } else if (std::filesystem::exists(seedPath) && seedPath.substr(seedPath.length() - 4) == ".csv") {
            // Load a single CSV file
            try {
                Individual graph = parseGraphFromCSV(seedPath);
                if (graphMatchesParameters(graph)) {
                    seedGraphs.push_back(graph);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error loading seed graph from " << seedPath << ": " << e.what() << std::endl;
            }
        } else {
            throw std::runtime_error("Invalid seed path: " + seedPath + ". Must be a directory or a .csv file.");
        }
        
        if (!seedGraphs.empty()) {
            std::cout << "Loaded " << seedGraphs.size() << " seed graphs" << std::endl;
        } else {
            throw std::runtime_error("No valid seed graphs found in " + seedPath);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Filesystem error: " + std::string(e.what()));
    }
}

Individual Grow::getSeedGraph() {
    if (seedGraphs.empty()) {
        throw std::runtime_error("No seed graphs available");
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, seedGraphs.size() - 1);
    return seedGraphs[dis(gen)];
}

bool Grow::hasSeedGraphs() const {
    return !seedGraphs.empty();
}

void Grow::initializePopulationWithSeeds(std::vector<Individual>& population, int populationSize) {
    if (seedGraphs.empty()) {
        throw std::runtime_error("No seed graphs available for initialization");
    }
    
    population.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> seedDis(0, seedGraphs.size() - 1);
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    // Calculate number of individuals from each source
    int numMutatedSeeds = static_cast<int>(populationSize * 0.9);  // 90% mutated seeds
    int numRandom = populationSize - numMutatedSeeds;              // 10% random
    
    // Add mutated seed graphs
    for (int i = 0; i < numMutatedSeeds; ++i) {
        // Get a random seed graph
        Individual seed = seedGraphs[seedDis(gen)];
        
        // Create a mutated version of the seed graph
        Individual mutated = mutate(seed, 0.1, k, alpha, beta, gen);  // Using 0.1 mutation rate for initialization
        
        // Ensure the mutated graph is valid
        if (!isValidGraph(mutated.graph, k)) {
            // If mutation produced an invalid graph, try again with a new seed
            --i;
            continue;
        }
        
        population.push_back(mutated);
    }
    
    // Add random individuals for the remaining 10%
    for (int i = 0; i < numRandom; ++i) {
        population.push_back(createIndividual(n, k, symmetry, alpha, beta));
    }
}

Individual Grow::parseGraphFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::string line;
    Graph graph(n);
    
    // Skip header lines until we find the adjacency list
    while (std::getline(file, line)) {
        if (line.find("Adjacency list:") != std::string::npos) {
            break;
        }
    }
    
    // Read the adjacency list
    for (int i = 0; i < n; ++i) {
        if (!std::getline(file, line)) {
            throw std::runtime_error("Unexpected end of file while reading adjacency list");
        }
        
        std::istringstream iss(line);
        int vertex;
        char colon;
        iss >> vertex >> colon;
        
        int neighbor;
        while (iss >> neighbor) {
            graph[vertex - 1].push_back(neighbor - 1); // Convert to 0-based indexing
        }
    }
    
    // Create and return the individual
    Individual ind;
    ind.graph = graph;
    ind.aspl = computeASPL(graph);
    ind.algebraicConnectivity = computeAlgebraicConnectivity(graph);
    ind.fitness = alpha * ind.aspl - beta * ind.algebraicConnectivity;
    return ind;
}

bool Grow::graphMatchesParameters(const Individual& graph) const {
    // Check if the graph has the correct number of vertices
    if (graph.graph.size() != n) {
        return false;
    }
    
    // Check if each vertex has the correct degree
    for (const auto& neighbors : graph.graph) {
        if (neighbors.size() != k) {
            return false;
        }
    }
    
    // Check symmetry if required
    if (symmetry > 0) {
        // Implement symmetry check here if needed
        // This would depend on how symmetry is defined in your system
    }
    
    return true;
} 