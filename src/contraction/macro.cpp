#include <vector>
#include <iostream>
#include <assert.h>

#include "util/timer.h"
#include "util/io.h"
#include "util/cast.h"

#include "query/graph.h"

using namespace Query;

/**
 * Generates a macroinstruction file for the customization
 * 
 * First argument: the upward dag
 * Second argument: the downward dag
 * Third argument: name of the macrocode file
 * Fourth argument: number of nodes in the graph
 * 
 * As output two files are written - the first named after the third parameter and containing the macrocode for customization and the second 'graph' containing the binary graph for all later phases.
 */
int main(int, char **argv) {
    int nodes = Util::stringToInt(argv[4]);
    std::vector<std::vector<Edge>> edgeData(nodes);

    std::vector<std::vector<int>> in(nodes);
    std::vector<std::vector<int>> out(nodes);

    for (int i = 1; i < 3; i++) {
        IOUtils::openFile(argv[i], [&] (std::ifstream &file) {
            int node = -1;
            int count = 0;
            int token;
            while (file >> token) {
                if (count == 0) {
                    count = token;
                    node++;
                } else {
                    if (i == 1) {
                        edgeData[node].push_back(Edge(token));
                        out[node].push_back(token);
                    } else {
                        edgeData[token].push_back(Edge(node));
                        in[node].push_back(token);
                    }
                    count--;
                }
            }
        });
    }
    Graph graph;
    Util::Timer timer;
    graph.build(edgeData);
    std::cout << timer.elapsedMilliseconds() << ",";

    timer.restart();
    IOUtils::writeFile(argv[3], [&] (std::ofstream &file) {
        for (int node = 0; node < graph.nodeCount(); node++) {
            unsigned int size = out[node].size();
            IOUtils::writeElementary(file, size);
            for (int target : out[node]) {
                int index = graph.getEdgeIndex(node, target);
                timer.pause();
                IOUtils::writeElementary(file, index);
                assert(graph.getEdge(index).target == target);
                timer.resume();
            }

            size = in[node].size();
            IOUtils::writeElementary(file, size);
            for (int source : in[node]) {
                int index = graph.getIncomingEdgeIndex(node, source);
                timer.pause();
                IOUtils::writeElementary(file, index);
                assert(graph.getIncomingEdge(index).target == source);
                timer.resume();
            }

            for (int target : out[node]) {
                for (int source : in[node]) {
                    if (source != target) {
                        int index1 = graph.getEdgeIndex(source, target);
                        int index2 = graph.getIncomingEdgeIndex(target, source);
                        timer.pause();
                        IOUtils::writeElementary(file, index1);
                        assert(graph.getEdge(index1).target == target);
                        IOUtils::writeElementary(file, index2);
                        assert(graph.getIncomingEdge(index2).target == source);
                        timer.resume();
                    }
                }
            }
        }
    }, std::ios::binary);
    std::cout << timer.elapsedMilliseconds() << "\n";

    IOUtils::writeFile("graph", [&] (std::ofstream &file) {
        graph.write(file);
    }, std::ios::binary);
}
