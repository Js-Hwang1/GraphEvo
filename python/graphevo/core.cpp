#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "GeneticAlgorithm.hpp"
#include "GeneticOperators.hpp"
#include "functions.hpp"
#include "GraphGenerator.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace py = pybind11;

// Debug print function (enabled only in DEBUG builds)
void debug_print(const std::string& msg) {
#ifdef DEBUG
    std::cerr << "[DEBUG] " << msg << std::endl;
#endif
}

// Convert C++ Graph (std::vector<std::vector<int>>) to a 2D numpy array.
// The array will have shape {n, max_row_length}, where shorter rows are padded with -1.
py::array_t<int> graph_to_numpy(const Graph& graph) {
    if (graph.empty()) {
        throw std::runtime_error("Cannot convert an empty graph to numpy array.");
    }
    
    int n = static_cast<int>(graph.size());
    int max_k = 0;
    for (const auto& edges : graph) {
        max_k = std::max(max_k, static_cast<int>(edges.size()));
    }
    
    if (max_k <= 0) {
        throw std::runtime_error("Invalid graph: no edges found.");
    }
    
    auto result = py::array_t<int>({ n, max_k });
    auto r = result.mutable_unchecked<2>();
    
    for (int i = 0; i < n; i++) {
        // Optional: Verify that each row is not excessively long.
        if (static_cast<int>(graph[i].size()) > max_k) {
            throw std::runtime_error("Row " + std::to_string(i) + " exceeds maximum expected size.");
        }
        for (int j = 0; j < max_k; j++) {
            r(i, j) = (j < static_cast<int>(graph[i].size())) ? graph[i][j] : -1;
        }
    }
    
    return result;
}

// Convert a 2D numpy array to a C++ Graph (std::vector<std::vector<int>>).
// The array is expected to have shape {n, m}. Any entry with a negative value is skipped.
Graph numpy_to_graph(py::array_t<int> array) {
    if (array.ndim() != 2) {
        throw std::runtime_error("Input array must be 2-dimensional");
    }
    
    auto r = array.unchecked<2>();
    int n = static_cast<int>(r.shape(0));
    int m = static_cast<int>(r.shape(1));
    if (n <= 0) {
        throw std::runtime_error("Input array must have at least one row");
    }
    
    Graph graph(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int val = r(i, j);
            if (val >= 0) {
                if (val >= n) {
                    throw std::runtime_error("Invalid node index in adjacency list: " + std::to_string(val));
                }
                graph[i].push_back(val);
            }
        }
    }
    
    return graph;
}

PYBIND11_MODULE(core, m) {
    m.doc() = R"pbdoc(
        GraphEvo: Genetic Algorithm for Graph Optimization
        ===============================================
        A Python interface for the GraphEvo C++ library that implements a genetic algorithm
        for optimizing graph structures with respect to various metrics such as average
        shortest path length and algebraic connectivity.
        
        .. currentmodule:: graphevo.core
        
        .. autosummary::
           :toctree: _generate

           GeneticAlgorithm
           Individual
           DiversityMetrics
           createIndividual
           computeASPL
           computeAlgebraicConnectivity
           minASPL
    )pbdoc";

    // Bind DiversityMetrics struct.
    py::class_<DiversityMetrics>(m, "DiversityMetrics", R"pbdoc(
        Metrics for measuring population diversity.
        
        Attributes:
            fitnessStdDev (float): Standard deviation of fitness values.
            fitnessRange (float): Range of fitness values.
            avgEdgeDiff (float): Average edge difference between individuals.
            pathDistDiversity (float): Path distance diversity metric.
    )pbdoc")
        .def(py::init<>())
        .def_readwrite("fitnessStdDev", &DiversityMetrics::fitnessStdDev)
        .def_readwrite("fitnessRange", &DiversityMetrics::fitnessRange)
        .def_readwrite("avgEdgeDiff", &DiversityMetrics::avgEdgeDiff)
        .def_readwrite("pathDistDiversity", &DiversityMetrics::pathDistDiversity);

    // Bind Individual struct.
    py::class_<Individual>(m, "Individual", R"pbdoc(
        Represents a single graph individual in the genetic algorithm.
        
        Attributes:
            graph (numpy.ndarray): Adjacency list representation of the graph.
            fitness (float): Fitness value of the individual.
            aspl (float): Average shortest path length.
            algebraicConnectivity (float): Algebraic connectivity (Fiedler value).
    )pbdoc")
        .def(py::init<>())
        .def_property("graph",
            [](const Individual& ind) { return graph_to_numpy(ind.graph); },
            [](Individual& ind, py::array_t<int> array) { ind.graph = numpy_to_graph(array); },
            R"pbdoc(
            Get or set the graph as a numpy array.
            
            The graph is represented as a 2D numpy array where each row represents
            a node's adjacency list. -1 indicates the end of the list.
            )pbdoc")
        .def_readwrite("fitness", &Individual::fitness)
        .def_readwrite("aspl", &Individual::aspl)
        .def_readwrite("algebraicConnectivity", &Individual::algebraicConnectivity);

    // Bind GeneticAlgorithm class.
    py::class_<GeneticAlgorithm>(m, "GeneticAlgorithm", R"pbdoc(
        Main genetic algorithm class for graph optimization.
        
        This class implements a genetic algorithm for optimizing graph structures
        with respect to various metrics.
    )pbdoc")
        .def(py::init<int, int, int, int, int, double, double, bool, double, double, bool>(),
            py::arg("n"),
            py::arg("k"),
            py::arg("symmetry"),
            py::arg("populationSize"),
            py::arg("generations"),
            py::arg("mutationRate"),
            py::arg("tolerance"),
            py::arg("useSeedGraphs") = false,
            py::arg("alpha") = 1.0,
            py::arg("beta") = 1.0,
            py::arg("computeDiversity") = false,
            R"pbdoc(
            Initialize the genetic algorithm.
            
            Args:
                n (int): Number of nodes in the graph.
                k (int): Target degree for each node.
                symmetry (int): Symmetry constraint (0: no symmetry, 1: symmetric).
                populationSize (int): Size of the population.
                generations (int): Number of generations to run.
                mutationRate (float): Probability of mutation.
                tolerance (float): Tolerance for convergence.
                useSeedGraphs (bool): Whether to use seed graphs.
                alpha (float): Weight for ASPL in fitness calculation.
                beta (float): Weight for algebraic connectivity in fitness calculation.
                computeDiversity (bool): Whether to compute diversity metrics.
            )pbdoc")
        .def("run", &GeneticAlgorithm::run, R"pbdoc(
            Run the genetic algorithm.
            
            Returns:
                Individual: The best individual found.
        )pbdoc")
        .def("getBestIndividual", &GeneticAlgorithm::getBestIndividual, R"pbdoc(
            Get the best individual from the current population.
            
            Returns:
                Individual: The best individual.
        )pbdoc")
        .def("initializePopulation", &GeneticAlgorithm::initializePopulation, R"pbdoc(
            Initialize the population.
        )pbdoc")
        .def("setSeedGraphDirectory", &GeneticAlgorithm::setSeedGraphDirectory, R"pbdoc(
            Set the directory containing seed graphs.
            
            Args:
                directory (str): Path to the directory containing seed graphs.
        )pbdoc");

    // Bind standalone functions.
    m.def("createIndividual", 
        [](int n, int k, int symmetry, double alpha, double beta) {
            if (n <= 0 || k <= 0 || k >= n) {
                throw std::runtime_error("Invalid parameters: n and k must be positive and k < n");
            }
            return createIndividual(n, k, symmetry, alpha, beta);
        },
        py::arg("n"),
        py::arg("k"),
        py::arg("symmetry"),
        py::arg("alpha") = 1.0,
        py::arg("beta") = 1.0,
        R"pbdoc(
        Create a new individual with specified parameters.
        
        Args:
            n (int): Number of nodes.
            k (int): Target degree.
            symmetry (int): Symmetry constraint.
            alpha (float): Weight for ASPL.
            beta (float): Weight for algebraic connectivity.
            
        Returns:
            Individual: A new individual.
        )pbdoc");

    m.def("computeASPL", [](py::array_t<int> array) {
        return computeASPL(numpy_to_graph(array));
    }, R"pbdoc(
    Compute the average shortest path length of a graph.
    
    Args:
        graph (numpy.ndarray): Adjacency list representation of the graph.
        
    Returns:
        float: Average shortest path length.
    )pbdoc");
    
    m.def("computeAlgebraicConnectivity", [](py::array_t<int> array) {
        return computeAlgebraicConnectivity(numpy_to_graph(array));
    }, R"pbdoc(
    Compute the algebraic connectivity (Fiedler value) of a graph.
    
    Args:
        graph (numpy.ndarray): Adjacency list representation of the graph.
        
    Returns:
        float: Algebraic connectivity.
    )pbdoc");
    
    m.def("minASPL", &minASPL, R"pbdoc(
    Get the theoretical minimum average shortest path length.
    
    Returns:
        float: Minimum possible ASPL.
    )pbdoc");

    m.def("generateSymmetricGraph", [](int n, int degree, int symmetry) {
        return graph_to_numpy(generateSymmetricGraph(n, degree, symmetry));
    }, R"pbdoc(
    Generate a symmetric graph with specified parameters.
    
    Args:
        n (int): Number of nodes.
        degree (int): Target degree for each node.
        symmetry (int): Symmetry constraint.
        
    Returns:
        numpy.ndarray: Adjacency list representation of the generated graph.
    )pbdoc");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}