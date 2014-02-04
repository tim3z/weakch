#include <vector>
#include <iostream>
#include <math.h>
#include <assert.h>

#include "util/io.h"

#include "graph.h"
#include "contraction.h"

using namespace Contraction;

/**
 * Contracts the given graph.
 * The node order is used implicitly by the IDs of the nodes.
 * 
 * First argument: a DIMACs .gr graph.
 * 
 * As output two files are written - up.dag and down.dag, each containing a DAG with either only up- or downward arcs (with respect to the node order). The arc set are all original arcs + all inserted shortcuts.
 */
int main(int, char **argv) {
    Graph graph;
    IOUtils::readGraph(argv[1], [&] (int nodes, int) {
        graph.setNodeCount(nodes);
    }, 
    [&] (std::vector<int> & args) {
        graph.addEdge(args[0], args[1]);
    });

	contract(graph);
	exportDAGs(graph);
}
