#include <iostream>
#include <vector>
#include <assert.h>
#include <unordered_set>

#include "util/io.h"
#include "util/timer.h"
#include "query/graph.h"

using namespace Query;

/**
 * ONLY FOR WEAK QUERY
 * Reordering arcs due to changes in the metric and recustomization
 * 
 * First parameter: binary file containing the graph
 * Second parameter: name of the file into which the performance stats about the change compuations should be written.
 *
 * Currently the result wont be written to anywhere (as this programm was only used to measure performance)
 */
int main(int, char **argv) {
	Graph graph;

	IOUtils::openFile(argv[1], [&] (std::ifstream &file) {
		graph.read(file);
	}, std::ios::binary);

	IOUtils::writeFile(argv[2], [&] (std::ofstream &stats) {
		stats << "build_in_ms,meta_in_ms\n";
		for (int i = 0; i < 1000; i++) {
			std::unordered_set<int> changed;
			std::unordered_set<int> incomingChanged;
			
			Util::Timer timer;
			double time = 0;
			IOUtils::readGraph("changes/" + std::to_string(i), [&] (int, int) {},
			[&] (std::vector<int> &input) {
				timer.restart();
				graph.setWeight(input[0], input[1], input[2], input[3]);
				time += timer.elapsedMilliseconds();
				changed.insert(input[0]);
				incomingChanged.insert(input[1]);
			});
			stats << time << ",";

			timer.restart();
			graph.regenerateMetaInformation(changed, incomingChanged);
			stats << timer.elapsedMilliseconds() << "\n";
		}
	});

	// IOUtils::writeFile("customized", [&] (std::ofstream &file) {
	// 	graph.write(file);
	// }, std::ios::binary);
}