#ifndef GROW_HPP
#define GROW_HPP

#include <vector>
#include <string>
#include <filesystem>
#include <memory>

// Forward declarations
struct Individual;
double evaluateFitness(const Individual& ind);
double calculateASPL(const Individual& ind);
double calculateAlgebraicConnectivity(const Individual& ind);
Individual createIndividual(int n, int k, int symmetry);

class Grow {
public:
    // Constructor
    Grow(int n, int k, int symmetry, double alpha, double beta);
    
    // Load seed graphs from a directory
    void loadSeedGraphs(const std::string& seedPath);
    
    // Get a seed graph that matches the current parameters
    Individual getSeedGraph();
    
    // Check if we have seed graphs available
    bool hasSeedGraphs() const;
    
    // Initialize population using seed graphs
    void initializePopulationWithSeeds(std::vector<Individual>& population, int populationSize);
    
private:
    int n;
    int k;
    int symmetry;
    double alpha;
    double beta;
    std::vector<Individual> seedGraphs;
    
    // Helper function to parse a graph from a CSV file
    Individual parseGraphFromCSV(const std::string& filename);
    
    // Helper function to check if a graph matches our parameters
    bool graphMatchesParameters(const Individual& graph) const;
};

#endif // GROW_HPP 