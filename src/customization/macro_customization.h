#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <utility>

#include "util/io.h"
#include "util/timer.h"
#include "Common/DataStructures/Container/KHeap.h"

#include "query/graph.h"

namespace Customization {

class MacroCustomization {
private:
	const std::string baseGraph;
	Query::Graph graph;
	bool dirty;

	std::vector<std::vector<int>> outgoingToLower;

	std::vector<std::vector<int>> out;
	std::vector<std::vector<int>> in;
	std::vector<std::vector<int>> indices;

	int uncontraced;

public:

	MacroCustomization(const std::string graphFile, const std::string instructionsFile, const int uncontraced = 0) : baseGraph(graphFile), dirty(false), uncontraced(uncontraced) {
		readGraph();
		readInstructions(instructionsFile);
	}

	const Query::Graph& getGraph() const {
		return graph;
	}

	void applyMetric(const std::string weights) {
		if (dirty) {
			readGraph();
		}
		IOUtils::readGraph(weights, [&] (int nodes, int) {
			(void) nodes; // fixing warning
			assert(nodes == graph.nodeCount());
			assert(nodes == in.size());
			assert(nodes == out.size());
			assert(nodes == indices.size());
		}, [&] (std::vector<int> & args) {
			graph.setWeight(args[0], args[1], args[2]);
			assert(graph.getEdge(args[0], args[1]).originalWeight == args[2]);
			assert(graph.getEdge(args[0], args[1]).weight == args[2]);
			assert(graph.getWeight(args[0], args[1]) == args[2]);
		});
		dirty = true;

		Util::Timer timer;
		for (int node = 0; node < graph.nodeCount(); node++) {
			customize(node);
		}
		std::cout << "customization took " << timer.elapsedMilliseconds() / 1000 << "s\n";

		#ifndef NDEBUG
			for (int i = 0; i < graph.edgeCount(); i++)	{
				Query::Edge & edge = graph.getEdge(i);
				assert(edge.weight <= edge.originalWeight);
			}
			IOUtils::readGraph(weights, [&] (int nodes, int) {
				(void) nodes; // fixing warning
				assert(nodes == graph.nodeCount());
			}, [&] (std::vector<int> & args) {
				assert(graph.getEdge(args[0], args[1]).originalWeight == args[2]);
				assert(graph.getWeight(args[0], args[1]) <= args[2]);
			});
		#endif
	}

	void change(const int from, const int to, const int newWeight) {
		Query::Edge &edge = graph.getEdge(from, to);
		if (edge.weight > newWeight) {
			decrease(from, to, newWeight);
		} else {
			if (edge.reason == -1) {
				increase(from, to, newWeight);
			}
			edge.originalWeight = newWeight;
		}
	}

private:

	void decrease(const int from, const int to, const int newWeight) {
		graph.setWeight(from, to, newWeight);
		Container::KHeap<2, int> queue(graph.nodeCount());
		int node = from < to ? from : to;
		
		queue.Update(node, node);

		while(!queue.Empty()) {
			queue.ExtractMin(node, node);
			forEachPotentialShortcut(node, [&] (Query::Edge &inEdge, Query::Edge &outEdge, Query::Edge &edge) {
				int sum = inEdge.weight + outEdge.weight;
	    		if (sum < edge.weight) {
					edge.weight = sum;
					edge.reason = node;

					int lowest = outEdge.target < inEdge.target ? outEdge.target : inEdge.target;
					queue.Update(lowest, lowest);
				}
		    });
		}
	}

	void increase(const int from, const int to, const int newWeight) {
		graph.setWeight(from, to, newWeight);
		int min = from < to ? from : to;

		Container::KHeap<2, int> changedEdgesLowerEnds(graph.nodeCount());
		changedEdgesLowerEnds.Update(min, min);
		Container::KHeap<2, int> toCustomize(graph.nodeCount());

		for (int target : outgoingToLower[from]) {
			toCustomize.Update(target, target);
		}

		while(!changedEdgesLowerEnds.Empty()) {
			int node;
			changedEdgesLowerEnds.ExtractMin(node, node);

			forEachPotentialShortcut(node, [&] (Query::Edge &inEdge, Query::Edge &outEdge, Query::Edge &edge) {
        		if (edge.reason == node) {
					edge.weight = edge.originalWeight;
					edge.reason = -1;

					min = inEdge.target < outEdge.target ? inEdge.target : outEdge.target;
					changedEdgesLowerEnds.Update(min, min);

					for (int target : outgoingToLower[inEdge.target]) {
						toCustomize.Update(target, target);
					}
        		}
		    });
		}

		while (!toCustomize.Empty()) {
			int node;
			toCustomize.ExtractMin(node, node);
			customize(node);
		}

	}

	inline void customize(const int node) {
		forEachPotentialShortcut(node, [&] (Query::Edge &inEdge, Query::Edge &outEdge, Query::Edge &edge) {
			int sum = inEdge.weight + outEdge.weight;
    		assert(inEdge.weight != INT_MAX);
			assert(outEdge.weight != INT_MAX);
    		if (sum < edge.weight) {
				edge.weight = sum;
				edge.reason = node;
			}
		});
	}

	template <class F>
	void forEachPotentialShortcut(const int node, F f) {
		int i = 0;
		for (int target : out[node]) {
			Query::Edge &outEdge = graph.getEdge(target);
	        for (int source : in[node]) {
	        	Query::Edge &inEdge = graph.getIncomingEdge(source);
	        	if (outEdge.target != inEdge.target) {
        			Query::Edge * edge;
    				int e = indices[node][i++];
        			if (inEdge.target < outEdge.target) {
        				edge = &graph.getEdge(e);
        				assert(outEdge.target == edge->target);
        			} else {
        				edge = &graph.getIncomingEdge(e);
        				assert(inEdge.target == edge->target);
        			}
        			f(inEdge, outEdge, *edge);
	        	}
	        }
	    }
	}

public:

	void exportGraph(const std::string targetFile) {
		IOUtils::writeFile(targetFile, [&] (std::ofstream &file) {
			graph.write(file);
		}, std::ios::binary);
	}

private:

	void readGraph() {
		IOUtils::openFile(baseGraph, [&] (std::ifstream &file) {
			graph.read(file);
		}, std::ios::binary);
		dirty = false;

		outgoingToLower.resize(graph.nodeCount());
		for (int node = 0; node < graph.nodeCount(); node++) {
			graph.forEachIncomingEdge(node, [&] (int source, int, int) {
				if (source > node) {
					outgoingToLower[source].push_back(node);
				}
			});
		}
	}

	void readInstructions(const std::string file) {
		IOUtils::openFile(file, [&] (std::ifstream &file) {
			out.resize(graph.nodeCount());
			in.resize(graph.nodeCount());
			indices.resize(graph.nodeCount());

			for (int node = 0; node < graph.nodeCount() - uncontraced; node++) {
				IOUtils::readVector(file, out[node]);
				IOUtils::readVector(file, in[node]);
				for (int target : out[node]) {
					Query::Edge &outEdge = graph.getEdge(target);
	                for (int source : in[node]) {
	                	Query::Edge &inEdge = graph.getIncomingEdge(source);
	                	if (outEdge.target != inEdge.target) {
	                		int forward;
		                    IOUtils::readElementary(file, forward);
		                    indices[node].push_back(forward);
	                	}
	                }
	            }
			}
		}, std::ios::binary);
	}

};
}