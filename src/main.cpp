#include "functions.hpp"
#include "GeneticAlgorithm.hpp"
#include "Helper.hpp"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) {
    try {
        // Initialize logging.
        initLog();
        logMessage("GraphEVO started.");

        // Default parameters.
        int n = 64, k = 3, symmetry = 1, populationSize = 1000, generations = 2000;
        double mutationRate = 0.01, tolerance = 0.0001;
        std::string seedDirectory;

        // Parse command-line arguments.
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
            } else if (strcmp(argv[i], "--seed-dir") == 0 && i + 1 < argc) {
                seedDirectory = argv[++i];
            } else {
                printUsage(argv[0]);
                return 1;
            }
        }
        
        // Compute theoretical lower ASPL and print header.
        const double theoreticalLowerASPL = minASPL(n, k);
        printHeader(n, k, symmetry, populationSize, generations, mutationRate, tolerance, theoreticalLowerASPL);
        logMessage("Parameters: n=" + std::to_string(n) +
                   ", k=" + std::to_string(k) +
                   ", symmetry=" + std::to_string(symmetry) +
                   ", populationSize=" + std::to_string(populationSize) +
                   ", generations=" + std::to_string(generations) +
                   ", mutationRate=" + std::to_string(mutationRate) +
                   ", tolerance=" + std::to_string(tolerance));

        // Create the GA engine with seed graph support if directory is provided
        bool useSeedGraphs = !seedDirectory.empty();
        GeneticAlgorithm ga(n, k, symmetry, populationSize, generations, mutationRate, tolerance, useSeedGraphs);
        
        if (useSeedGraphs) {
            ga.setSeedGraphDirectory(seedDirectory);
            logMessage("Using seed graphs from directory: " + seedDirectory);
        }
        
        // Initialize the population.
        ga.initializePopulation();
        logMessage("Population initialized with " + std::to_string(populationSize) + " individuals.");
        
        auto startTime = high_resolution_clock::now();
        
        // Run the GA loop.
        bool found = ga.run();
        
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime);
        cout << "GA completed in " << duration.count() << " ms.\n";
        logMessage("GA completed in " + std::to_string(duration.count()) + " ms.");
        
        // If no acceptable individual was found, report the best.
        if (!found) {
            Individual best = ga.getBestIndividual();
            cout << "Best individual:\n"
                 << "  ASPL = " << best.aspl << "\n"
                 << "  Algebraic Connectivity = " << best.algebraicConnectivity << "\n";
            outputToCSV(best, theoreticalLowerASPL, symmetry);
            logMessage("Best individual: ASPL=" + std::to_string(best.aspl) +
                       ", Algebraic Connectivity=" + std::to_string(best.algebraicConnectivity));
        }
        
        closeLog();
        return 0;
    } catch (std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        if (logFile.is_open()) {
            logMessage("Exception: " + std::string(ex.what()));
            closeLog();
        }
        return 1;
    }
}
