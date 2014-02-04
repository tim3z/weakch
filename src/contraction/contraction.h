#pragma once

#include "util/timer.h"

namespace Contraction {

template<class Graph>
void contract(Graph &graph, const int uncontracted = 0) {
	std::cout << "contracting\n";
	std::cout << "leaving " << uncontracted << " nodes uncontracted\n";

	int percent = 0;
	int shortcuts = 0;

	Util::Timer timer;
	for (int vertex = 0; vertex < graph.nodeCount() - uncontracted; vertex++) {
		if (100 * vertex / graph.nodeCount() > percent) {
			percent = 100 * vertex / graph.nodeCount();
			std::cout << percent << "% " << std::flush;
		}

		graph.removeEdges(vertex); // this removes only the edges in the targets/sources lists, not on the current node

		graph.forEachOutgoingEdgeTarget(vertex, [&] (int target) {
			graph.forEachIncomingEdgeSource(vertex, [&] (int source) {
				if (graph.addEdge(source, target)) {
					shortcuts++;
				}
			});
		});
	}
	for (int node = graph.nodeCount() - uncontracted; node < graph.nodeCount(); node++) {
        graph.removeEdges(node);
    }

	std::cout << "done \nadded " << shortcuts << " shortcuts - took " << timer.elapsedMilliseconds() / 1000 << "s\n";
}

template<class Graph>
void exportDAGs(const Graph &graph) {
	IOUtils::writeFile("up.dag", [&] (std::ofstream &up) {
	IOUtils::writeFile("down.dag", [&] (std::ofstream &down) {
		for (int vertex = 0; vertex < graph.nodeCount(); vertex++) {
			up << graph.edgeCount(vertex);
			graph.forEachOutgoingEdgeTarget(vertex, [&] (int target) {
				assert(target > vertex);
				up << " " << target;
			});
			up << "\n";
			
			down << graph.incomingEdgeCount(vertex);
			graph.forEachIncomingEdgeSource(vertex, [&] (int source) {
				assert(source > vertex);
				down << " " << source;
			});
			down << "\n";
		}
	});});
}

template<class Graph>
void exportEliminationTree(const Graph &graph, std::string filename) {
	IOUtils::writeFile(filename, [&] (std::ofstream &tree) {
        for (int vertex = 0; vertex < graph.nodeCount(); vertex++) {
            int lowestRank = graph.nodeCount();
            graph.forEachEdgeTarget(vertex, [&] (int target) {
                assert(target > vertex);
                if (target < lowestRank) {
                    lowestRank = target;
                }
            });
            tree << (lowestRank != graph.nodeCount() ? lowestRank : -1) << "\n";
        }
    });
}
}