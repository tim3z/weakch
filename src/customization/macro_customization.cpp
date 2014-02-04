#include "macro_customization.h"
#include "util/random.h"

/**
 * Introduces an arbitrary metric to a preprocessed graph and generates a graph ready for fast queries.
 *
 * First parameter: a graph file (binary format) as generated for example in macro.cpp
 * Second parameter: the macroinstruction file as generated for example in macro.cpp
 * Third parameter: a file in the DIMACs .gr format containing the metric
 * Fourth parameter: name for output graph file
 *
 * Creates a binary file containing the graph ready for queries.
 */
int main(int, char **argv) {
	Customization::MacroCustomization customization(argv[1], argv[2]);
	customization.applyMetric(argv[3]);
	customization.exportGraph(argv[4]);

	// changing some arc weights - comment out the above export if you want to use this - the arc reordering will break the macroinstructions
	// IOUtils::writeFile(argv[5], [&] (std::ofstream &stats) {
	// 	stats << "recustomization_in_ms\n";
	// 	Util::Random random(1372074269);
	// 	for (int i = 0; i < 1000; ++i) {
	// 		int from = random.getRandomInt(customization.getGraph().nodeCount());
	// 		int offset = random.getRandomInt(customization.getGraph().edgeCount(from));
	// 		const Query::Edge &edge = customization.getGraph().getEdgeOfNode(from, offset);
	// 		assert(edge.weight != INT_MAX);

	// 		Common::Tools::Timer timer;
	// 		customization.change(from, edge.target, random.getRandomInt(edge.weight * 2) + 1); // no zero edges
	// 		stats << timer.ElapsedMilliseconds() << "\n";

	// 		IOUtils::writeFile("changes/" + std::to_string(i), [&] (std::ofstream &file) {
	// 			for (auto arc : customization.getChanges()) {
	// 				const Query::Edge &edge = customization.getGraph().getEdge(arc.first, arc.second);
	// 				assert(edge.weight != INT_MAX);
	// 				file << "a " << arc.first << " " << arc.second << " " << edge.weight << " " << edge.reason << "\n";
	// 			}
	// 		});
	// 	}
	// });
}