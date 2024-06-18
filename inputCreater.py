import random
import os

def create_graph_file(vertices, edges, edge_list, filename):
    with open(filename, 'w') as file:
        # Write number of vertices and edges
        file.write(f"{vertices} {len(edge_list)}\n")
        
        # Write each edge
        for edge in edge_list:
            file.write(f"{edge[0]} {edge[1]}\n")
    print(f"Graph file created: {filename}")

# Number of vertices and edges
vertices = 1500
edges = 8000

# Generate a list of edges
edge_list = set()
while len(edge_list) < edges:
    x = random.randint(1, vertices)
    y = random.randint(1, vertices)
    if x != y:  # No self-loops
        edge_list.add((x, y))

# Convert set to list
edge_list = list(edge_list)
filename = os.getcwd() + "/graph" + str(vertices) + "_" + str(edges) + ".txt"

# Create the graph file
create_graph_file(vertices, edges, edge_list, filename)
