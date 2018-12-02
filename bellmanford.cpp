
/**
 * Patrick Nappa's (pnappa) implementation of Bellman-Ford.
 * This is soooorta based on github.com/sunnlo/BellmanFord
 * but rewritten to be c++11-ish, and supporting string vertex labels
 * plus adjacency lists (for when you have a lot of vertices)
 */

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include <unordered_map>

// to allow symbol lookup (int -> symbol)
std::vector<std::string> symbol_map;
// allow key lookup (string -> int)
std::unordered_map<std::string, int64_t> key_map;

using AdjacencyList = std::vector<std::vector<std::pair<int64_t, int64_t>>>;
// don't use int::max, because we have to add distances sometimes...
constexpr int64_t INF = std::numeric_limits<int64_t>::max();

#define MICROGET(x)                                                            \
  std::chrono::duration_cast<std::chrono::microseconds>(x).count()
typedef std::chrono::high_resolution_clock hrclock;

std::unordered_map<int64_t, std::vector<int64_t>> fastest_paths;

/* returns the number representing this symbol, creating one if necessary */
int64_t symbol_to_key(const std::string &s) {
  if (key_map.count(s) == 0) {
    key_map.emplace(s, key_map.size());
    // symbol map is assumed to be reserved
    symbol_map[key_map.size() - 1] = s;
  }
  return key_map[s];
}

/* read in the file and spit out the adjacency list */
AdjacencyList read_file(const std::string &filename) {

  // file format is:
  // NUMVERTS
  // VERT1 NUMEDGES VERT2 COST VERTK COST ...
  // ..
  //
  // The file assumes each vertex has a line (even if they have no edges to xor
  // from them)

  AdjacencyList ret;

  std::ifstream inputf(filename, std::ifstream::in);
  if (!inputf.good()) {
    throw std::runtime_error("fuck me, file didn't open");
  }

  int64_t num_vertices;
  inputf >> num_vertices;

  if (num_vertices <= 0)
    throw std::runtime_error("provide a positive number you mong");

  ret.resize(num_vertices);
  symbol_map.resize(num_vertices);

  for (int64_t i = 0; i < num_vertices; ++i) {
    std::string this_vert_str;
    size_t num_edges;

    inputf >> this_vert_str;
    inputf >> num_edges;

    int64_t cVert = symbol_to_key(this_vert_str);

    for (size_t edgen = 0; edgen < num_edges; ++edgen) {
      std::string term_vertex_str;
      inputf >> term_vertex_str;
      int64_t cost;
      inputf >> cost;

      int64_t cTerm = symbol_to_key(term_vertex_str);
      ret[cVert].push_back({cTerm, cost});
    }
  }

  return ret;
}

// pretty print
void print_adjacency(const AdjacencyList &adjacency_list) {
  for (size_t ind = 0; ind < adjacency_list.size(); ++ind) {
    std::cout << symbol_map[ind] << "->";
    for (auto r : adjacency_list.at(ind)) {
      std::cout << symbol_map[r.first] << ":" << r.second << ",";
    }
    std::cout << std::endl;
  }
}

void bellman_ford(AdjacencyList &adjlist, std::vector<int64_t> &dlist) {
  // first vertex has distance 0
  dlist[0] = 0;
  fastest_paths.emplace(0, std::vector<int64_t>({}));

  bool has_change;
  // iterate n-1 times
  for (size_t i = 0; i < adjlist.size() - 1; ++i) {
    has_change = false;
    // for each vertex
    for (int64_t vertNum = 0; vertNum < static_cast<int64_t>(adjlist.size());
         ++vertNum) {
      // skip this vertex, as it's currently unreachable.
      if (dlist[vertNum] == INF)
        continue;
      int64_t rcvVert, rcvWeight;
      for (std::pair<int64_t, int64_t> rcv : adjlist[vertNum]) {
        std::tie(rcvVert, rcvWeight) = rcv;

        // we've found a faster path
        if (dlist[vertNum] + rcvWeight < dlist[rcvVert]) {
          has_change = true;
          dlist[rcvVert] = dlist[vertNum] + rcvWeight;

          // replace the path to this vertex to be pathTovertNum + vertNum
          // we MUST have a path to that prev vertex, otherwise the path weight
          // doesn't make sense
          assert(fastest_paths.count(vertNum) == 1);
          std::vector<int64_t> prev = (*fastest_paths.find(vertNum)).second;
          prev.push_back(vertNum);
          fastest_paths.emplace(rcvVert, prev);
        }
      }
    }

    // no better paths found, finish.
    if (!has_change)
      return;
  }
}

void dump_distances(std::string outfile, const std::vector<int64_t> &dist) {
  std::ofstream outputf(outfile, std::ofstream::out);
  for (size_t i = 0; i < dist.size(); ++i) {
    int64_t cost = dist[i];
    outputf << symbol_map[i] << ":";
    if (cost == INF) {
      outputf << "INF";
    } else {
      outputf << cost;
    }
    outputf << std::endl;
  }
  outputf.flush();
  outputf.close();
}

void dump_paths(
    std::string outfile,
    const std::unordered_map<int64_t, std::vector<int64_t>> &paths) {
  std::ofstream outputf(outfile, std::ofstream::out);
  for (auto pairP : paths) {
    outputf << symbol_map[pairP.first] << ":";

    size_t ctr = 0;
    for (auto pathNode : pairP.second) {
      outputf << symbol_map[pathNode];
      if (ctr != pairP.second.size() - 1)
        outputf << ",";
      ++ctr;
    }
    outputf << std::endl;
  }
  outputf.flush();
  outputf.close();
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "usage:" << argv[0] << " inputfile\n";
    return -1;
  }

  auto start = hrclock::now();

  constexpr char outfile[] = "output.txt";

  // read the edges from file
  AdjacencyList adjacency_list = read_file(argv[1]);

  // initialise the distances to be infinite
  std::vector<int64_t> distance_list(symbol_map.size(), INF);

  auto end = hrclock::now();

  std::cout << "loading graph took " << MICROGET(end - start) << "ms"
            << std::endl;

  start = hrclock::now();

  // do bellman ford (assumes graph hasn't got a negative cycle)
  bellman_ford(adjacency_list, distance_list);

  end = hrclock::now();

  std::cout << "bellman-ford took " << MICROGET(end - start) << "ms"
            << std::endl;

  start = hrclock::now();
  dump_distances(outfile, distance_list);
  dump_paths("paths.txt", fastest_paths);
  end = hrclock::now();
  std::cout << "writing paths/costs took " << MICROGET(end - start) << "ms"
            << std::endl;
}
