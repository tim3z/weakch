#include <iostream>
#include <vector>
#include <unordered_map>
#include <climits>
#include <assert.h>

#include "util/io.h"
#include "util/timer.h"

namespace Customization {

struct Edge {
	int weight = INT_MAX;
	int reason = -1;
};

struct Node {
	std::unordered_map<int, Edge> in;
	std::unordered_map<int, Edge> out;
};

struct Graph {
    std::vector<Node> nodes;
	
	int edgeCount() {
		int sum = 0;
		for (auto node : nodes) {
			sum += node.out.size() + node.in.size();
		}
		return sum;
	}
	
	void addEdge(const int from, const int to) {
		if (to > from) {
			nodes[from].out[to];
		} else {
			nodes[to].in[from];
		}
	}
	
	void setWeight(const int from, const int to, const int weight, const int reason = -1) {
		Edge *edge;
		if (to > from) {
			edge = &nodes[from].out.at(to);
		} else {
			edge = &nodes[to].in.at(from);
		}
		edge->weight = weight;
		edge->reason = reason;
	}
	
	int getWeight(const int from, const int to) {
		if (to > from) {
			return nodes[from].out.at(to).weight;
		} else {
			return nodes[to].in.at(from).weight;
		}
	}
};

}

using namespace Customization;

/**
 * DEPRECATED 
 * slow customization (without macrocode)
 */
int main(int, char **argv) {
    Graph graph;

	IOUtils::readGraph(argv[1], [&] (int nodes, int) {
		graph.nodes.resize(nodes);
		for (int i = 2; i < 4; i++) {
			IOUtils::openFile(argv[i], [&] (std::ifstream &file) {
				int node = -1;
				int count = 0;
				int token;
				while (file >> token) {
					if (count == 0) {
						count = token;
						node++;
					} else {
						if (i == 2) {
							graph.addEdge(node, token);
						} else {
							graph.addEdge(token, node);
						}
						count--;
					}
				}
			});
		}
	}, 
	[&] (std::vector<int> & args) {
		graph.setWeight(args[0], args[1], args[2]);
	});

	Util::Timer timer;
	for (unsigned int i = 0; i < graph.nodes.size(); i++) {
		for (auto &out : graph.nodes[i].out) {
			for (auto &in : graph.nodes[i].in) {
				if (in.first != out.first) {
					int firstWeight = in.second.weight;
					int secondWeight = out.second.weight;
					int sum = firstWeight + secondWeight;
					assert(firstWeight != INT_MAX);
					assert(secondWeight != INT_MAX);
					if (sum < graph.getWeight(in.first, out.first)) {
						graph.setWeight(in.first, out.first, sum, i);
					}
				}
			}
		}
	}
	std::cout << "customization took " << timer.elapsedMilliseconds() / 1000 << "s\n";
	
    IOUtils::writeFile("customized.gr", [&] (std::ofstream &out) {
		out << "p sp " << graph.nodes.size() << " " << graph.edgeCount() << "\n";
		for (unsigned int node = 0; node < graph.nodes.size(); node++) {
			for (auto &edge : graph.nodes[node].out) {
				out << "a " << node << " " << edge.first << " " << edge.second.weight << " " << edge.second.reason << "\n";
			}
			for (auto &edge : graph.nodes[node].in) {
				out << "a " << edge.first << " " << node << " " << edge.second.weight << " " << edge.second.reason << "\n";
			}
		}
    });
}