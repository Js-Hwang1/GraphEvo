import unittest
import numpy as np
import graphevo

class TestGraphEvo(unittest.TestCase):
    def setUp(self):
        self.n = 10
        self.k = 3
        self.symmetry = 1
        self.population_size = 50
        self.generations = 10
        self.mutation_rate = 0.1
        self.tolerance = 1e-6
        self.ga = graphevo.GeneticAlgorithm(
            self.n, self.k, self.symmetry,
            self.population_size, self.generations,
            self.mutation_rate, self.tolerance
        )

    def test_initialization(self):
        self.assertEqual(self.ga.n, self.n)
        self.assertEqual(self.ga.k, self.k)
        self.assertEqual(self.ga.symmetry, self.symmetry)
        self.assertEqual(self.ga.populationSize, self.population_size)
        self.assertEqual(self.ga.generations, self.generations)
        self.assertEqual(self.ga.mutationRate, self.mutation_rate)
        self.assertEqual(self.ga.tolerance, self.tolerance)

    def test_create_individual(self):
        ind = graphevo.createIndividual(self.n, self.k, self.symmetry)
        self.assertIsInstance(ind.graph, np.ndarray)
        self.assertEqual(ind.graph.shape[0], self.n)
        self.assertEqual(ind.graph.shape[1], self.k)
        self.assertGreater(ind.fitness, 0)
        self.assertGreater(ind.aspl, 0)
        self.assertGreater(ind.algebraicConnectivity, 0)

    def test_compute_aspl(self):
        ind = graphevo.createIndividual(self.n, self.k, self.symmetry)
        aspl = graphevo.computeASPL(ind.graph)
        self.assertGreater(aspl, 0)
        self.assertLessEqual(aspl, graphevo.minASPL(self.n, self.k))

    def test_compute_algebraic_connectivity(self):
        ind = graphevo.createIndividual(self.n, self.k, self.symmetry)
        ac = graphevo.computeAlgebraicConnectivity(ind.graph)
        self.assertGreater(ac, 0)

    def test_diversity_metrics(self):
        ga = graphevo.GeneticAlgorithm(
            self.n, self.k, self.symmetry,
            self.population_size, self.generations,
            self.mutation_rate, self.tolerance,
            computeDiversity=True
        )
        ga.run()
        best_ind = ga.getBestIndividual()
        self.assertGreater(best_ind.fitness, 0)

if __name__ == '__main__':
    unittest.main() 