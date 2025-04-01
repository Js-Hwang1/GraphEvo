#include "Helper.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>


using namespace std;

std::ofstream logFile;

bool isValidGraph(const Graph &g, int degree) {
    // Check degrees and symmetry.
    for (size_t i = 0; i < g.size(); i++) {
        // Check if vertex i has the correct degree.
        if (static_cast<int>(g[i].size()) != degree) {
            return false;
        }
        // Check symmetry: for each neighbor j of i, ensure i is in g[j].
        for (int j : g[i]) {
            // Make sure j is a valid vertex.
            if (j < 0 || j >= static_cast<int>(g.size())) {
                return false;
            }
            // Check if i exists in the neighbor list of j.
            if (std::find(g[j].begin(), g[j].end(), i) == g[j].end()) {
                return false;
            }
        }
    }
    
    // Check connectivity using BFS.
    int n = g.size();
    std::vector<bool> visited(n, false);
    std::queue<int> q;
    q.push(0);
    visited[0] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : g[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    // If any vertex is not visited, the graph is disconnected.
    for (bool flag : visited) {
        if (!flag)
            return false;
    }
    return true;
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
    
    // Header information.
    outFile << "Theoretical lower bound:" << fixed << setprecision(20) << theoreticalMinASPL <<endl;
    outFile << "minASPL: " << fixed << setprecision(20) << ind.aspl << endl;
    outFile << "symmetry(g): " << symmetry  << endl;
    outFile << "Algebraic Connectivity: " << fixed << setprecision(6) << ind.algebraicConnectivity << endl;
    outFile << "Adjacency list:" << endl;
    
    // Output the adjacency list.
    int n = ind.graph.size();
    for (int i = 0; i < n; i++) {
        // Convert vertex index to 1-indexed.
        outFile << (i + 1) << ":";
        for (int neighbor : ind.graph[i]) {
            outFile << " " << (neighbor + 1);
        }
        outFile << endl;
    }
    outFile.close();
}

void initLog() {
    logFile.open("GraphEVO.log", std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error: Unable to open log file." << std::endl;
    }
}

void logMessage(const std::string &msg) {
    if (logFile.is_open()) {
        logFile << msg << std::endl;
    }
}

void closeLog() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  -n <int>       Total number of vertices (default: 64)\n"
              << "  -k <int>       Regular graph degree (default: 3)\n"
              << "  -s <int>       Symmetry parameter (default: 1)\n"
              << "  -p <int>       Population size (default: 200)\n"
              << "  -g <int>       Number of generations (default: 10000)\n"
              << "  -m <double>    Mutation rate (default: 0.1)\n"
              << "  -t <double>    Tolerance (default: 0.0001)\n";
}