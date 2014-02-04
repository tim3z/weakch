#pragma once

#include <vector>
#include <assert.h>
#include <stdexcept> 
#include <algorithm> 
#include <climits>
#include <unordered_set>

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

struct Meta {
	int causeNode = -1;
	int count = 0;
};


class Graph {
private:
	std::vector<Node> nodes;
	std::vector<Edge> edges;
	std::vector<Edge> incomingEdges;
	std::vector<std::vector<Meta>> metaData;
	std::vector<std::vector<Meta>> incomingMetaData;

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
    void forEachOutgoingEdge(int node, const std::vector<bool> &searchSpace, F f) const {
    	assert(node < nodeCount() && node >= 0);

    	int i = nodes[node].edgesIndex;
    	for (const Meta &meta : metaData[node]) {
			if (meta.causeNode == -1 || !searchSpace[meta.causeNode]) {
				int border = i + meta.count;
				while (i < border) {
					f(edges[i].target, edges[i].weight, meta.causeNode);
					i++;
				}
			} else {
				i += meta.count;
			}
		}
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
    void forEachIncomingEdge(int node, const std::vector<bool> &searchSpace, F f) const {
    	assert(node < nodeCount() && node >= 0);

		int i = nodes[node].incomingEdgesIndex;
    	for (const Meta &meta : incomingMetaData[node]) {
			if (meta.causeNode == -1 || !searchSpace[meta.causeNode]) {
				int border = i + meta.count;
				while (i < border) {
					f(incomingEdges[i].target, incomingEdges[i].weight, meta.causeNode);
					i++;
				}
			} else {
				i += meta.count;
			}
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
    	Edge &edge = getEdge(from, to);
    	edge.weight = weight;
    	edge.originalWeight = weight;
    	edge.reason = reason;
    	Edge &bEdge = getIncomingEdge(to, from);
    	bEdge.weight = weight;
    	bEdge.reason = reason;
    	bEdge.originalWeight = weight;
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
				edges[edgeId++] = edge;
				incomingData[edge.target].push_back(edge);
				incomingData[edge.target].back().target = nodeId;
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

	void generateMetaInformation() {
		metaData.clear();
		incomingMetaData.clear();

		metaData.resize(nodeCount());
		incomingMetaData.resize(nodeCount());

		for (int node = 0; node < nodeCount(); node++) {
			generateMetaInformationFor(node);
			generateIncomingMetaInformationFor(node);
		}
	}

	void regenerateMetaInformation(std::unordered_set<int> changed, std::unordered_set<int> incomingChanged) {
		for (int node : changed) {
			generateMetaInformationFor(node);
		}
		for (int node : incomingChanged) {
			generateIncomingMetaInformationFor(node);
		}
	}
private:

	inline void generateMetaInformationFor(const int nodeId) {
		metaData[nodeId].clear();
		int i = nodes[nodeId].edgesIndex;
		const int end = nodes[nodeId+1].edgesIndex;
		std::sort(edges.begin() + i, edges.begin() + end, [] (const Edge &first, const Edge &second) {
			return first.reason < second.reason;
		});

		Meta meta;
		meta.count = 0;
		meta.causeNode = edges[i].reason;
		while (i < end) {
			if (meta.causeNode != edges[i].reason) {
				metaData[nodeId].push_back(meta);
				meta.count = 0;
				meta.causeNode = edges[i].reason;
			}
			meta.count++;
			i++;
		}
		metaData[nodeId].push_back(meta);
	}
	inline void generateIncomingMetaInformationFor(const int nodeId) {
		incomingMetaData[nodeId].clear();
		int i = nodes[nodeId].incomingEdgesIndex;
		const int end = nodes[nodeId+1].incomingEdgesIndex;
		std::sort(incomingEdges.begin() + i, incomingEdges.begin() + end, [] (const Edge &first, const Edge &second) {
			return first.reason < second.reason;
		});

		Meta meta;
		meta.count = 0;
		meta.causeNode = incomingEdges[i].reason;
		while (i < end) {
			if (meta.causeNode != incomingEdges[i].reason) {
				incomingMetaData[nodeId].push_back(meta);
				meta.count = 0;
				meta.causeNode = incomingEdges[i].reason;
			}
			meta.count++;
			i++;
		}
		incomingMetaData[nodeId].push_back(meta);
	}
public:

	void write(std::ofstream &file) {
		IOUtils::writeVector(file, nodes);
		IOUtils::writeVector(file, edges);
		IOUtils::writeVector(file, incomingEdges);
		IOUtils::writeVector(file, metaData, [&] (std::ofstream &f, std::vector<Meta> &metas) {
			IOUtils::writeVector(f, metas);
		});
		IOUtils::writeVector(file, incomingMetaData, [&] (std::ofstream &f, std::vector<Meta> &metas) {
			IOUtils::writeVector(f, metas);
		});
	}

	void read(std::ifstream &file) {
		IOUtils::readVector(file, nodes);
		IOUtils::readVector(file, edges);
		IOUtils::readVector(file, incomingEdges);
		IOUtils::readVector(file, metaData, [&] (std::ifstream &f, std::vector<Meta> &metas) {
			IOUtils::readVector(f, metas);
		});
		IOUtils::readVector(file, incomingMetaData, [&] (std::ifstream &f, std::vector<Meta> &metas) {
			IOUtils::readVector(f, metas);
		});
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