#pragma once

#include <vector>
#include <assert.h>
#include <stdexcept> 
#include <algorithm> 
#include <climits>

#include "util/vector_utils.h"
#include "util/io.h"

namespace Query {

struct Node {
	int edgesIndex = -1;
	int incomingEdgesIndex = -1;
};

struct Edge {
	Edge(const int target = -1, const int weight = INT_MAX, const int reason = -1) : target(target), weight(weight), reason(reason), originalWeight(weight) {}
	int target;
	int weight;
	int reason;
	int originalWeight;
};

class Graph {
private:
	std::vector<Node> nodes;
	std::vector<Edge> edges;
	std::vector<Edge> incomingEdges;

	void resize(const int nodesSize, const int edgesSize) {
		nodes.resize(nodesSize + 1); edges.resize(edgesSize); incomingEdges.resize(edgesSize);
	}
	
public:
	inline int nodeCount() const {
        return nodes.size() - 1;
    }

    inline int edgeCount() const {
        return edges.size();
    }

    int edgeCount(const int node) const {
    	assert(node < nodeCount() && node >= 0);
		return nodes[node + 1].edgesIndex - nodes[node].edgesIndex;
    }

    int incomingEdgeCount(const int node) const {
    	assert(node < nodeCount() && node >= 0);
		return nodes[node + 1].incomingEdgesIndex - nodes[node].incomingEdgesIndex;
    }

    template<class F> 
    void forEachOutgoingEdge(int node, F f) const {
    	assert(node < nodeCount() && node >= 0);
		int i = nodes[node].edgesIndex;
		int end = nodes[node + 1].edgesIndex;
		while (i < end) {
			f(edges[i].target, edges[i].weight, edges[i].reason);
			i++;
		}
    }

    template<class F> 
    void forEachIncomingEdge(int node, F f) const {
    	assert(node < nodeCount() && node >= 0);
		int i = nodes[node].incomingEdgesIndex;
		int end = nodes[node + 1].incomingEdgesIndex;
		while (i < end) {
			f(incomingEdges[i].target, incomingEdges[i].weight, incomingEdges[i].reason);
			i++;
		}
    }

    int getWeight(const int from, const int to) {
    	return getEdge(from, to).weight;
    }

    void setWeight(const int from, const int to, const int weight, const int reason = -1) {
    	if (from < to) {
    		Edge &edge = getEdge(from, to);
	    	edge.weight = weight;
	    	edge.originalWeight = weight;
	    	edge.reason = reason;
	    } else {
	    	Edge &bEdge = getIncomingEdge(to, from);
	    	bEdge.weight = weight;
	    	bEdge.reason = reason;
	    	bEdge.originalWeight = weight;
	    }
    }

  //   template<class F>
  //   void forEachEdge(int node, F f, bool backward = false) const {
  //   	backward ? forEachIncomingEdge(node, f) : forEachOutgoingEdge(node, f);
  //   }
	
	void build(std::vector<std::vector<Edge>> &edgeData) {
		int edgeCount = 0;
		for (std::vector<Edge> &vector : edgeData) {
			edgeCount += vector.size();
		}
		resize(edgeData.size(), edgeCount);
		std::vector<std::vector<Edge>> incomingData(edgeData.size());

		int edgeId = 0;
		for (unsigned int nodeId = 0; nodeId < edgeData.size(); nodeId++) {
			nodes[nodeId].edgesIndex = edgeId;
			for (Edge &edge : edgeData[nodeId]) {
				if ((int) nodeId < edge.target) {
					edges[edgeId++] = edge;
				} else {
					incomingData[edge.target].push_back(edge);
					incomingData[edge.target].back().target = nodeId;
				}
			}
		}
		nodes[edgeData.size()].edgesIndex = edgeId;

		edgeId = 0;
		for (unsigned int nodeId = 0; nodeId < incomingData.size(); nodeId++) {
			nodes[nodeId].incomingEdgesIndex = edgeId;
			for (Edge &edge : incomingData[nodeId]) {
				incomingEdges[edgeId++] = edge;
		 	}
		}
		nodes[edgeData.size()].incomingEdgesIndex = edgeId;
	}

	void write(std::ofstream &file) {
		IOUtils::writeVector(file, nodes);
		IOUtils::writeVector(file, edges);
		IOUtils::writeVector(file, incomingEdges);
	}

	void read(std::ifstream &file) {
		IOUtils::readVector(file, nodes);
		IOUtils::readVector(file, edges);
		IOUtils::readVector(file, incomingEdges);
	}

	inline Edge& getEdge(const int i) {
		return edges[i];
	}

	inline Edge& getIncomingEdge(const int i) {
		return incomingEdges[i];
	}

	Edge& getEdge(const int from, const int to) {
		return edges[getEdgeIndex(from, to)];
	}

	Edge& getIncomingEdge(const int from, const int to) {
		return incomingEdges[getIncomingEdgeIndex(from, to)];
	}

	const Edge& getEdge(const int from, const int to) const {
		return edges[getEdgeIndex(from, to)];
	}

	const Edge& getIncomingEdge(const int from, const int to) const {
		return incomingEdges[getIncomingEdgeIndex(from, to)];
	}

	int getEdgeIndex(const int from, const int to) const {
		assert(from < nodeCount() && from >= 0);
		assert(to < nodeCount() && to >= 0);

		int i = nodes[from].edgesIndex;
		int end = nodes[from+1].edgesIndex;
		while (i < end) {
			if (edges[i].target == to) {
				return i;
			}
			i++;
		}
		throw std::runtime_error("Edge from " + std::to_string(from) + " to " + std::to_string(to) + " does not exist"); 
	}

	int getIncomingEdgeIndex(const int from, const int to) const {
		assert(from < nodeCount() && from >= 0);
		assert(to < nodeCount() && to >= 0);

		int i = nodes[from].incomingEdgesIndex;
		int end = nodes[from+1].incomingEdgesIndex;
		while (i < end) {
			if (incomingEdges[i].target == to) {
				return i;
			}
			i++;
		}
		throw std::runtime_error("incomingedge from " + std::to_string(from) + " to " + std::to_string(to) + " does not exist"); 
	}

	const Edge& getEdgeOfNode(const int node, const int offset) const {
		return edges[nodes[node].edgesIndex + offset];
	}
};
}