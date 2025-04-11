#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <queue>
#include "../include/GraphGenerator.hpp"
#include "../include/Helper.hpp"

struct GraphAnalysis {
    std::vector<std::vector<int>> graph;
    int diameter;
    double clustering_coefficient;
    std::vector<int> degree_distribution;
};

// Function to compute ASPL without using the external implementation
double computeASPL(const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    long long total_distance = 0;
    int total_pairs = 0;
    
    for (int i = 0; i < n; i++) {
        std::vector<int> distances(n, -1);
        std::queue<int> q;
        q.push(i);
        distances[i] = 0;
        
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            
            for (int v : graph[u]) {
                if (distances[v] == -1) {
                    distances[v] = distances[u] + 1;
                    q.push(v);
                }
            }
        }
        
        for (int j = i + 1; j < n; j++) {
            if (distances[j] != -1) {
                total_distance += distances[j];
                total_pairs++;
            }
        }
    }
    
    return static_cast<double>(total_distance) / total_pairs;
}

// Function to compute graph diameter
int computeDiameter(const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    int diameter = 0;
    
    for (int i = 0; i < n; i++) {
        std::vector<int> distances(n, -1);
        std::queue<int> q;
        q.push(i);
        distances[i] = 0;
        
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            
            for (int v : graph[u]) {
                if (distances[v] == -1) {
                    distances[v] = distances[u] + 1;
                    q.push(v);
                    diameter = std::max(diameter, distances[v]);
                }
            }
        }
    }
    
    return diameter;
}

// Function to compute clustering coefficient
double computeClusteringCoefficient(const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    double total_coefficient = 0.0;
    
    for (int i = 0; i < n; i++) {
        int k = graph[i].size();
        if (k < 2) continue;
        
        int triangles = 0;
        for (int j = 0; j < k; j++) {
            for (int l = j + 1; l < k; l++) {
                int u = graph[i][j];
                int v = graph[i][l];
                if (std::find(graph[u].begin(), graph[u].end(), v) != graph[u].end()) {
                    triangles++;
                }
            }
        }
        
        double max_triangles = k * (k - 1) / 2.0;
        total_coefficient += (max_triangles > 0) ? triangles / max_triangles : 0;
    }
    
    return total_coefficient / n;
}

// Function to compute edge overlap between two graphs
double computeEdgeOverlap(const std::vector<std::vector<int>>& graph1, 
                         const std::vector<std::vector<int>>& graph2) {
    int n = graph1.size();
    int common_edges = 0;
    int total_edges = 0;
    
    for (int i = 0; i < n; i++) {
        std::set<int> edges1(graph1[i].begin(), graph1[i].end());
        std::set<int> edges2(graph2[i].begin(), graph2[i].end());
        
        std::vector<int> intersection;
        std::set_intersection(edges1.begin(), edges1.end(),
                            edges2.begin(), edges2.end(),
                            std::back_inserter(intersection));
        
        common_edges += intersection.size();
        total_edges += edges1.size();
    }
    
    return static_cast<double>(common_edges) / total_edges;
}

// Function to analyze common subgraph patterns
void analyzeCommonPatterns(const std::vector<std::vector<std::vector<int>>>& graphs) {
    int n = graphs[0].size();
    std::map<std::vector<int>, int> pattern_frequency;
    
    // Analyze local patterns around each vertex
    for (const auto& graph : graphs) {
        for (int i = 0; i < n; i++) {
            std::vector<int> pattern;
            // Include vertex degree
            pattern.push_back(graph[i].size());
            // Include degrees of neighbors
            for (int neighbor : graph[i]) {
                pattern.push_back(graph[neighbor].size());
            }
            std::sort(pattern.begin() + 1, pattern.end());
            pattern_frequency[pattern]++;
        }
    }
    
    // Print most common patterns
    std::cout << "\nMost Common Local Patterns:\n";
    std::vector<std::pair<std::vector<int>, int>> sorted_patterns(
        pattern_frequency.begin(), pattern_frequency.end());
    std::sort(sorted_patterns.begin(), sorted_patterns.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (int i = 0; i < std::min(5, static_cast<int>(sorted_patterns.size())); i++) {
        std::cout << "Pattern " << i + 1 << " (Frequency: " 
                  << sorted_patterns[i].second << "): ";
        for (int val : sorted_patterns[i].first) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}

// Function to analyze path distribution
void analyzePathDistribution(const std::vector<std::vector<std::vector<int>>>& graphs) {
    std::map<int, int> path_length_distribution;
    int total_paths = 0;
    
    for (const auto& graph : graphs) {
        int n = graph.size();
        for (int i = 0; i < n; i++) {
            std::vector<int> distances(n, -1);
            std::queue<int> q;
            q.push(i);
            distances[i] = 0;
            
            while (!q.empty()) {
                int u = q.front();
                q.pop();
                
                for (int v : graph[u]) {
                    if (distances[v] == -1) {
                        distances[v] = distances[u] + 1;
                        path_length_distribution[distances[v]]++;
                        total_paths++;
                        q.push(v);
                    }
                }
            }
        }
    }
    
    std::cout << "\nPath Length Distribution:\n";
    for (const auto& [length, count] : path_length_distribution) {
        double percentage = (static_cast<double>(count) / total_paths) * 100;
        std::cout << "Length " << length << ": " << std::fixed << std::setprecision(2) 
                  << percentage << "%\n";
    }
}

// Function to analyze local structures
void analyzeLocalStructures(const std::vector<std::vector<std::vector<int>>>& graphs) {
    std::map<std::string, int> motif_counts;
    
    for (const auto& graph : graphs) {
        int n = graph.size();
        for (int i = 0; i < n; i++) {
            // Analyze 2-hop neighborhood
            std::set<int> neighborhood;
            neighborhood.insert(i);
            for (int v : graph[i]) {
                neighborhood.insert(v);
                for (int w : graph[v]) {
                    neighborhood.insert(w);
                }
            }
            
            // Create a signature for this local structure
            std::vector<int> degrees;
            for (int v : neighborhood) {
                degrees.push_back(graph[v].size());
            }
            std::sort(degrees.begin(), degrees.end());
            
            std::stringstream ss;
            for (int d : degrees) {
                ss << d << " ";
            }
            motif_counts[ss.str()]++;
        }
    }
    
    std::cout << "\nMost Common Local Structures:\n";
    std::vector<std::pair<std::string, int>> sorted_motifs(
        motif_counts.begin(), motif_counts.end());
    std::sort(sorted_motifs.begin(), sorted_motifs.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (int i = 0; i < std::min(5, static_cast<int>(sorted_motifs.size())); i++) {
        std::cout << "Structure " << i + 1 << " (Frequency: " 
                  << sorted_motifs[i].second << "): " 
                  << sorted_motifs[i].first << "\n";
    }
}

// Function to analyze edge roles
void analyzeEdgeRoles(const std::vector<std::vector<std::vector<int>>>& graphs) {
    int n = graphs[0].size();
    std::vector<std::vector<int>> edge_frequency(n, std::vector<int>(n, 0));
    
    // Count how often each edge appears
    for (const auto& graph : graphs) {
        for (int i = 0; i < n; i++) {
            for (int j : graph[i]) {
                if (i < j) {  // Count each edge only once
                    edge_frequency[i][j]++;
                    edge_frequency[j][i]++;
                }
            }
        }
    }
    
    // Classify edges based on their frequency
    int total_edges = 0;
    int critical_edges = 0;  // Edges that appear in >90% of optimal graphs
    int common_edges = 0;    // Edges that appear in >50% of optimal graphs
    int rare_edges = 0;      // Edges that appear in <10% of optimal graphs
    
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (edge_frequency[i][j] > 0) {
                total_edges++;
                double frequency = static_cast<double>(edge_frequency[i][j]) / graphs.size();
                if (frequency > 0.9) critical_edges++;
                else if (frequency > 0.5) common_edges++;
                else if (frequency < 0.1) rare_edges++;
            }
        }
    }
    
    std::cout << "\nEdge Role Analysis:\n";
    std::cout << "Total unique edges: " << total_edges << "\n";
    std::cout << "Critical edges (>90% frequency): " << critical_edges 
              << " (" << (critical_edges * 100.0 / total_edges) << "%)\n";
    std::cout << "Common edges (>50% frequency): " << common_edges 
              << " (" << (common_edges * 100.0 / total_edges) << "%)\n";
    std::cout << "Rare edges (<10% frequency): " << rare_edges 
              << " (" << (rare_edges * 100.0 / total_edges) << "%)\n";
}

// Function to analyze symmetry
void analyzeSymmetry(const std::vector<std::vector<std::vector<int>>>& graphs) {
    std::cout << "\nSymmetry Analysis:\n";
    
    for (size_t i = 0; i < graphs.size(); i++) {
        const auto& graph = graphs[i];
        int n = graph.size();
        int symmetric_pairs = 0;
        
        // Check for vertex symmetry
        for (int v = 0; v < n; v++) {
            for (int w = v + 1; w < n; w++) {
                // Check if vertices v and w have isomorphic neighborhoods
                if (graph[v].size() == graph[w].size()) {
                    std::vector<int> v_neighbors = graph[v];
                    std::vector<int> w_neighbors = graph[w];
                    std::sort(v_neighbors.begin(), v_neighbors.end());
                    std::sort(w_neighbors.begin(), w_neighbors.end());
                    
                    bool is_symmetric = true;
                    for (size_t j = 0; j < v_neighbors.size(); j++) {
                        if (graph[v_neighbors[j]].size() != graph[w_neighbors[j]].size()) {
                            is_symmetric = false;
                            break;
                        }
                    }
                    
                    if (is_symmetric) symmetric_pairs++;
                }
            }
        }
        
        double symmetry_score = static_cast<double>(symmetric_pairs) / (n * (n - 1) / 2);
        std::cout << "Graph " << i << " symmetry score: " 
                  << std::fixed << std::setprecision(4) << symmetry_score << "\n";
    }
}

void analyzeOptimalGraphs(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::vector<GraphAnalysis> analyses;
    std::vector<std::vector<std::vector<int>>> all_graphs;
    std::string line;
    std::vector<std::vector<int>> current_adjacency;
    int vertex_count = 64; // For 32_3.csv
    bool reading_adjacency = false;

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Start of a new graph
        if (line.find("Graph") != std::string::npos) {
            if (!current_adjacency.empty()) {
                all_graphs.push_back(current_adjacency);
            }
            current_adjacency.clear();
            current_adjacency.resize(vertex_count);
            continue;
        }

        // Skip other metadata lines
        if (line.find("Global minASPL:") != std::string::npos ||
            line.find("minASPL:") != std::string::npos ||
            line.find("Adjacency list:") != std::string::npos) {
            continue;
        }

        // Parse adjacency line in format "1: 2 3 4"
        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos) {
            try {
                // Extract vertex number (1-based)
                int vertex = std::stoi(line.substr(0, colon_pos)) - 1; // Convert to 0-based

                // Extract neighbors
                std::string neighbors_str = line.substr(colon_pos + 1);
                std::istringstream iss(neighbors_str);
                std::vector<int> neighbors;
                int neighbor;
                while (iss >> neighbor) {
                    neighbors.push_back(neighbor - 1); // Convert to 0-based indexing
                }
                if (vertex >= 0 && vertex < vertex_count) {
                    current_adjacency[vertex] = neighbors;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << "\n";
                continue;
            }
        }
    }

    // Add the last graph if not empty
    if (!current_adjacency.empty()) {
        all_graphs.push_back(current_adjacency);
    }

    if (all_graphs.empty()) {
        std::cerr << "No valid graphs found in the file.\n";
        return;
    }

    // Analyze each graph
    for (const auto& graph : all_graphs) {
        GraphAnalysis analysis;
        analysis.graph = graph;
        analysis.diameter = computeDiameter(graph);
        analysis.clustering_coefficient = computeClusteringCoefficient(graph);
        analyses.push_back(analysis);
    }

    // Print analysis results
    std::cout << "\nAnalysis of Optimal Graphs:\n";
    std::cout << "Total number of optimal graphs: " << analyses.size() << "\n\n";

    // Compute and print average properties
    double avg_diameter = 0.0;
    double avg_clustering = 0.0;
    for (const auto& analysis : analyses) {
        avg_diameter += analysis.diameter;
        avg_clustering += analysis.clustering_coefficient;
    }
    avg_diameter /= analyses.size();
    avg_clustering /= analyses.size();

    std::cout << "Average Diameter: " << std::fixed << std::setprecision(6) << avg_diameter << "\n";
    std::cout << "Average Clustering Coefficient: " << avg_clustering << "\n\n";

    // Analyze edge overlap between graphs
    std::cout << "Edge Overlap Analysis:\n";
    double total_overlap = 0.0;
    int overlap_count = 0;
    std::vector<std::vector<double>> overlap_matrix(all_graphs.size(), std::vector<double>(all_graphs.size()));
    
    for (size_t i = 0; i < all_graphs.size(); i++) {
        for (size_t j = i + 1; j < all_graphs.size(); j++) {
            double overlap = computeEdgeOverlap(all_graphs[i], all_graphs[j]);
            overlap_matrix[i][j] = overlap_matrix[j][i] = overlap;
            total_overlap += overlap;
            overlap_count++;
        }
    }
    double avg_overlap = total_overlap / overlap_count;
    std::cout << "Average Edge Overlap: " << avg_overlap * 100 << "%\n";

    // Find clusters of similar graphs
    std::cout << "\nGraph Similarity Clusters (>80% edge overlap):\n";
    std::vector<bool> visited(all_graphs.size(), false);
    for (size_t i = 0; i < all_graphs.size(); i++) {
        if (!visited[i]) {
            std::vector<size_t> cluster;
            cluster.push_back(i);
            visited[i] = true;
            
            for (size_t j = i + 1; j < all_graphs.size(); j++) {
                if (!visited[j] && overlap_matrix[i][j] > 0.8) {
                    cluster.push_back(j);
                    visited[j] = true;
                }
            }
            
            if (cluster.size() > 1) {
                std::cout << "Cluster with " << cluster.size() << " graphs: ";
                for (size_t idx : cluster) {
                    std::cout << idx << " ";
                }
                std::cout << "\n";
            }
        }
    }

    // Analyze common patterns
    analyzeCommonPatterns(all_graphs);

    // Add new analyses
    analyzePathDistribution(all_graphs);
    analyzeLocalStructures(all_graphs);
    analyzeEdgeRoles(all_graphs);
    analyzeSymmetry(all_graphs);
}

int main() {
    analyzeOptimalGraphs("seeds.csv");
    return 0;
} 
