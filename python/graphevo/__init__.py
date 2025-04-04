"""
GraphEvo: Genetic Algorithm for Graph Optimization
===============================================

A Python interface for the GraphEvo C++ library that implements a genetic algorithm
for optimizing graph structures with respect to various metrics such as average
shortest path length and algebraic connectivity.

Example:
    >>> from graphevo import GeneticAlgorithm, createIndividual
    >>> ga = GeneticAlgorithm(n=10, k=3, symmetry=0, populationSize=100, generations=1000)
    >>> best_individual = ga.run()
    >>> print(f"Best fitness: {best_individual.fitness}")
"""

from typing import Any, Optional, Tuple, Union
import numpy as np

from .core import (
    GeneticAlgorithm,
    Individual,
    DiversityMetrics,
    createIndividual,
    computeASPL,
    computeAlgebraicConnectivity,
    minASPL
)

__version__ = "0.1.0"
__author__ = "Js-Hwang1"
__email__ = "Junsung.K.Hwang@gmail.com"

__all__ = [
    "GeneticAlgorithm",
    "Individual",
    "DiversityMetrics",
    "createIndividual",
    "computeASPL",
    "computeAlgebraicConnectivity",
    "minASPL"
]

# Type aliases for better documentation
Graph = np.ndarray  # 2D numpy array representing adjacency list
Fitness = float
ASPL = float
AlgebraicConnectivity = float

def __dir__() -> list[str]:
    """Return the list of public attributes."""
    return __all__ 