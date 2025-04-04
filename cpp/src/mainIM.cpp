// mainIM.cpp
#include "functions.hpp"          
#include <mpi.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// Helper: serialize an Individual into a vector<int>
// Assumes that the individual's graph is stored as a vector of n rows, each with exactly k neighbors.
static std::vector<int> serializeIndividual(const Individual &ind, int n, int k) {
    std::vector<int> data;
    data.reserve(n * k);
    for (int i = 0; i < n; i++) {
        // Assuming vertices are numbered 0..n-1 and each row has exactly k integers.
        for (int j = 0; j < k; j++) {
            data.push_back(ind.graph[i][j]);
        }
    }
    return data;
}

// Helper: deserialize an Individual from a vector<int>
// Reconstructs the graph assuming it is stored as n*k integers.
static Individual deserializeIndividual(const std::vector<int> &data, int n, int k) {
    Individual ind;
    ind.graph.resize(n);
    auto it = data.begin();
    for (int i = 0; i < n; i++) {
        ind.graph[i].resize(k);
        for (int j = 0; j < k; j++) {
            ind.graph[i][j] = *it;
            ++it;
        }
    }
    // Recompute fitness metrics.
    ind.aspl = computeASPL(ind.graph);
    ind.algebraicConnectivity = computeAlgebraicConnectivity(ind.graph);
    // Assuming lambda = 1.
    ind.fitness = ind.aspl - ind.algebraicConnectivity;
    return ind;
}

// Helper: integrate a migrated individual into the population by replacing the worst individual if better.
static void integrateMigration(GeneticAlgorithm &ga, const Individual &incoming) {
    // Find the worst individual (highest fitness) in the population.
    int worstIndex = 0;
    double worstFitness = ga.population[0].fitness;
    for (size_t i = 1; i < ga.population.size(); i++) {
        if (ga.population[i].fitness > worstFitness) {
            worstFitness = ga.population[i].fitness;
            worstIndex = i;
        }
    }
    // Replace the worst individual if the incoming one is better.
    if (incoming.fitness < worstFitness) {
        ga.population[worstIndex] = incoming;
    }
}

int main(int argc, char* argv[]) {
    // Initialize MPI.
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    try {
        // Initialize logging.
        initLog();
        logMessage("GraphEVO Island Model started on process " + to_string(rank));

        // Default GA parameters.
        int n = 32, k = 3, symmetry = 1, populationSize = 100, totalGenerations = 100;
        double mutationRate = 0.1, tolerance = 0.0001;
        int migrationInterval = 10; // Migrate every 10 generations.

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
                totalGenerations = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
                mutationRate = atof(argv[++i]);
            } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                tolerance = atof(argv[++i]);
            } else if (strcmp(argv[i], "-mi") == 0 && i + 1 < argc) {
                migrationInterval = atoi(argv[++i]);
            } else {
                printUsage(argv[0]);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 1;
            }
        }

        // Compute theoretical lower ASPL.
        const double theoreticalLowerASPL = minASPL(n, k);
        if (rank == 0) {
            printHeader(n, k, symmetry, populationSize, totalGenerations, mutationRate, tolerance, theoreticalLowerASPL);
        }
        {
            ostringstream paramLog;
            paramLog << "Parameters: n=" << n
                     << ", k=" << k
                     << ", symmetry=" << symmetry
                     << ", populationSize=" << populationSize
                     << ", totalGenerations=" << totalGenerations
                     << ", mutationRate=" << mutationRate
                     << ", tolerance=" << tolerance
                     << ", migrationInterval=" << migrationInterval;
            logMessage(paramLog.str());
        }

        // Create the GA engine for this island.
        // Note: The GeneticAlgorithm constructor uses totalGenerations as the planned evolution length.
        GeneticAlgorithm ga(n, k, symmetry, populationSize, totalGenerations, mutationRate, tolerance);
        ga.initializePopulation();
        logMessage("Population initialized with " + to_string(populationSize) + " individuals on process " + to_string(rank));

        auto startTime = high_resolution_clock::now();

        int remainingGenerations = totalGenerations;
        // Run evolution in chunks of migrationInterval generations.
        while (remainingGenerations > 0) {
            int currentChunk = std::min(migrationInterval, remainingGenerations);
            // Set the number of generations to run in this chunk.
            ga.generations = currentChunk;
            // Run GA for the current chunk.
            bool found = ga.run(); // run() returns true if an acceptable graph is found.
            remainingGenerations -= currentChunk;

            // If there are still generations remaining, perform migration.
            if (remainingGenerations > 0) {
                // Each island sends its best individual to the next island in a ring.
                Individual bestLocal = ga.getBestIndividual();
                // Serialize the best individual into a vector<int>.
                vector<int> bestData = serializeIndividual(bestLocal, n, k);
                int dataSize = bestData.size();

                int dest = (rank + 1) % size;
                int source = (rank - 1 + size) % size;

                // Send the size.
                MPI_Send(&dataSize, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                // Receive the size from the source.
                int incomingSize = 0;
                MPI_Recv(&incomingSize, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Send the serialized data.
                MPI_Send(bestData.data(), dataSize, MPI_INT, dest, 1, MPI_COMM_WORLD);
                // Receive the incoming data.
                vector<int> incomingData(incomingSize);
                MPI_Recv(incomingData.data(), incomingSize, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Deserialize the incoming individual.
                Individual incomingInd = deserializeIndividual(incomingData, n, k);
                // Integrate the migrated individual into the local population.
                integrateMigration(ga, incomingInd);
                logMessage("Migration completed at a migration interval on process " + to_string(rank));
            }
        }

        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime);
        cout << "Process " << rank << ": GA completed in " << duration.count() << " ms." << endl;
        logMessage("GA completed in " + to_string(duration.count()) + " ms on process " + to_string(rank));

        // Obtain the best individual from this island.
        Individual bestLocal = ga.getBestIndividual();
        double localFitness = bestLocal.fitness; // Assuming lower fitness is better.

        // Gather the best fitness from all islands via MPI_Reduce.
        double globalBestFitness;
        MPI_Reduce(&localFitness, &globalBestFitness, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            cout << "Global best fitness: " << globalBestFitness << endl;
            logMessage("Global best fitness: " + to_string(globalBestFitness));
            // Optionally, output the best graph.
            outputToCSV(bestLocal, theoreticalLowerASPL, symmetry);
        }

        closeLog();
        MPI_Finalize();
        return 0;
    } catch (std::exception &ex) {
        cerr << "Exception on process " << rank << ": " << ex.what() << endl;
        if (logFile.is_open()) {
            logMessage("Exception: " + string(ex.what()));
            closeLog();
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
}