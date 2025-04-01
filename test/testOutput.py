#!/usr/bin/env python3
import sys
from collections import deque

def parse_csv(filename):
    """
    Parses the CSV file where each line is in the format:
    "vertex: neighbor1 neighbor2 neighbor3 ..."
    
    Returns a dictionary mapping each vertex (as int) to a set of its neighbors.
    """
    graph = {}
    with open(filename, 'r') as f:
        for line in f:
            # Remove whitespace and ignore empty lines.
            line = line.strip()
            if not line:
                continue
            # Expecting line like: "1: 2 22 32"
            try:
                vertex_part, neighbors_part = line.split(":", 1)
                vertex = int(vertex_part.strip())
                # Split neighbors by whitespace; ignore empty strings.
                neighbors = set(int(n) for n in neighbors_part.strip().split() if n)
                graph[vertex] = neighbors
            except ValueError:
                print(f"Error parsing line: {line}")
    return graph

def is_k_regular(graph):
    """
    Checks if the graph is k-regular (all vertices have the same degree).
    Returns (True, k) if yes; otherwise (False, None).
    """
    if not graph:
        return False, None
    # Use first vertex's degree as target
    degrees = [len(neighbors) for neighbors in graph.values()]
    k = degrees[0]
    for d in degrees:
        if d != k:
            return False, None
    return True, k

def is_symmetric(graph):
    """
    Checks if the graph is symmetric: for every edge u-v,
    v must be in the neighbor set of u and vice versa.
    """
    for u, neighbors in graph.items():
        for v in neighbors:
            # If vertex v is not in graph or u is not in v's neighbor list
            if v not in graph or u not in graph[v]:
                return False
    return True

def is_connected(graph):
    """
    Checks if the graph is connected using BFS.
    Returns True if all vertices are reachable from an arbitrary starting vertex.
    """
    if not graph:
        return False
    start = next(iter(graph))
    visited = set([start])
    queue = [start]
    while queue:
        current = queue.pop(0)
        for neighbor in graph[current]:
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)
    return len(visited) == len(graph)

def bfs(graph, start):
    """
    Performs a breadth-first search (BFS) starting from 'start'.
    Returns a dictionary of distances from start to each vertex.
    If a vertex is unreachable, its distance will remain None.
    """
    distances = {v: None for v in graph}
    distances[start] = 0
    queue = deque([start])
    
    while queue:
        current = queue.popleft()
        for neighbor in graph[current]:
            if distances[neighbor] is None:
                distances[neighbor] = distances[current] + 1
                queue.append(neighbor)
    return distances

def compute_aspl(graph):
    """
    Computes the Average Shortest Path Length (ASPL) using BFS.
    Assumes the graph is connected. If not connected, only considers pairs 
    that are reachable.
    """
    total_distance = 0
    count = 0
    vertices = list(graph.keys())
    n = len(vertices)
    
    for i, v in enumerate(vertices):
        distances = bfs(graph, v)
        for w in vertices:
            if w != v and distances[w] is not None:
                total_distance += distances[w]
                count += 1
        # Optional: print progress for large graphs
        if (i+1) % 100 == 0:
            print(f"Processed {i+1}/{n} vertices for BFS...")
    
    if count == 0:
        return float('inf')
    return total_distance / count

def main():
    # Use filename from command line or default to 'output.csv'
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        filename = "output.csv"
    
    graph = parse_csv(filename)
    if not graph:
        print("Graph is empty or could not be parsed.")
        sys.exit(1)
    
    regular, k = is_k_regular(graph)
    symmetric = is_symmetric(graph)
    connected = is_connected(graph)
    
    print(f"Graph has {len(graph)} vertices.")
    if regular:
        print(f"Graph is k-regular with k = {k}.")
    else:
        print("Graph is not k-regular.")
    
    if symmetric:
        print("Graph is symmetric.")
    else:
        print("Graph is not symmetric.")
    
    if connected:
        print("Graph is connected.")
    else:
        print("Graph is not connected.")
    
    # Compute ASPL only if the graph is symmetric and connected.
    if symmetric and connected:
        aspl = compute_aspl(graph)
        print(f"Average Shortest Path Length (ASPL): {aspl:.4f}")
    else:
        print("Skipping ASPL computation since the graph is not symmetric and connected.")

if __name__ == "__main__":
    main()