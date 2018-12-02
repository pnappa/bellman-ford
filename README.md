# Bellman-Ford, in C++
Another Bellman-Ford implementation, but this time in C++.

This is pretty high-performance, and uses human-representable vertex names (i.e. strings), so you're not constrained to integers. The input graph is an adjacency list, in a custom space delimited format (see File Format).

For the input graph provided (`graph.txt`), it takes 0.2 seconds to perform Bellman-Ford, excluding setup/cleaning up time (around a second runtime overall).

Compile with `make`, and run on the sample input `graph.txt` with `./bellmanford graph.txt`.

## File format
### Input graph
The input graph is an adjacency list, in the following format.

```
NUMBERVERTICES
V0 NUMEDGES V2 COST2 V3 COST3 ...
V5 NUMEDGES V3 COST3 ...
...
```

The first row denotes the source vertex, such that the program will evaluate all shortest from this vertex to all others.

A small example is as follows:
```
3
nodea 1 nodeb 55
nodec 2 nodeb 20 nodea 13
nodeb 0
```

You *must* include a row for every vertex, even if it has no outgoing edges.


### Output
Two files are created - `output.txt` and `paths.txt`.

`output.txt` contains the costs for each edge, whilst `paths.txt` contains the shortest paths to reach each vertex.
