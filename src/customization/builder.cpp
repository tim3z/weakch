#include <iostream>
#include <vector>
#include <assert.h>

#include "util/io.h"
#include "util/timer.h"
#include "query/graph.h"

using namespace Query;

/**
 * Converts a DIMACs .gr graph (first parameter) into the binary graph format of this project. 
 * The name of the output file is given in the second parameter.
 */
int main(int, char **argv) {
	std::vector<std::vector<Edge>> edgeData;
	Graph graph;

	IOUtils::readGraph(argv[1], [&] (int nodes, int) {
		edgeData.resize(nodes);
	},
	[&] (std::vector<int> &input) {
		edgeData[input[0]].push_back(Edge(input[1], input[2], input[3]));
	});

	Util::Timer timer;
	graph.build(edgeData);
	std::cout << timer.elapsedMilliseconds() << ",";
	timer.restart();
	graph.generateMetaInformation();
	std::cout << timer.elapsedMilliseconds() << "\n";

	IOUtils::writeFile(argv[2], [&] (std::ofstream &file) {
		graph.write(file);
	}, std::ios::binary);
}