#ifndef HELPER_HPP
#define HELPER_HPP

#include "functions.hpp"

#include <iostream>
#include <fstream>
#include <string>

extern std::ofstream logFile;

bool isValidGraph(const Graph &g, int degree);

// Prints header information for the GA run.
void printHeader(int n, int k, int symmetry, int populationSize, int generations, double mutationRate, double tolerance, double theoreticalLowerASPL);

// Outputs the best individual's graph and metrics to a CSV file.
void outputToCSV(const Individual &ind, double theoreticalMinASPL, int symmetry);

// Initializes the log file.
void initLog();

// Logs a message to the log file.
void logMessage(const std::string &msg);

// Closes the log file.
void closeLog();

// Prints the usage information.
void printUsage(const char* progName);

#endif // HELPER_HPP