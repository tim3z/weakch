#include <iostream>
#include <vector>

#include "util/io.h"
#include "util/random.h"
#include "graph.h"
#include "ch_query.h"

using namespace Query;

/**
 * Running (a lot of) queries
 *
 * First parameter: binary file containing the precomputed graph
 * Second parameter: name of the file where the query statistics should be written into
 */
int main(int, char **argv) {
	Graph graph;

	IOUtils::openFile(argv[1], [&] (std::ifstream &file) {
		graph.read(file);
	}, std::ios::binary);

    IOUtils::writeFile(argv[2], [&] (std::ofstream &log) {
		CHQuery algorithm(graph, log);

		Util::Random random(1372074269);
		for (int i = 0; i < 10000; ++i) {
			int from = random.getRandomInt(graph.nodeCount());
			int to = random.getRandomInt(graph.nodeCount());
			std::cout << "shortest path from " << from << " to " << to << ": " << algorithm.shortestPath(from, to) << "\n";
		}
    });
}
