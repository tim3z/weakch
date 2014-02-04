#include <vector>
#include <iostream>
#include <stdexcept> 
#include <math.h>
#include <assert.h>

#include "util/io.h"
#include "undirected_graph.h"
#include "contraction.h"

using namespace Contraction;

/**
 * Contracts the given graph as if every arc was an edge.
 * The node order is used implicitly by the IDs of the nodes.
 * 
 * First argument: a DIMACs .gr graph.
 * Optional second argument: name for elimination tree file - defaults to 'eliminationtree'
 * 
 * As output a file containing the elimination tree is written (for each node a line with the id of the parent node)
 */
int main(int argc, char **argv) {
    UndirectedGraph graph;
    IOUtils::readGraph(argv[1], [&] (int nodes, int) {
        graph.setNodeCount(nodes);
    }, 
    [&] (std::vector<int> & args) {
        graph.addEdge(args[0], args[1]);
    });

    contract(graph);
	exportEliminationTree(graph, argc == 2 ? "eliminationtree" : argv[2]);
}
