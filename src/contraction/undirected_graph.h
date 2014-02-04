#pragma once

#include <vector>
#include <unordered_set>

namespace Contraction {

class UndirectedGraph {
private: 
    std::vector<std::unordered_set<int>> nodes;

public:
    UndirectedGraph(const int nodeCount = 0) : nodes(nodeCount) {}

    void setNodeCount(int count) {
        nodes.resize(count);
    }

    inline int nodeCount() const {
        return nodes.size();
    }

    int edgeCount() const {
        int count = 0;
        for (const auto &node : nodes) {
            count += node.size();
        }
        return count / 2;
    }

    int edgeCount(const int node) const {
        return nodes[node].size();
    }
    inline int incomingEdgeCount(const int node) const {
        return edgeCount(node);
    }

    bool addEdge(const int from, const int to) {
        if (from != to && !edgeExists(from, to)) {
            nodes[from].insert(to);
            nodes[to].insert(from);
            return true;
        }
        return false;
    }

    bool edgeExists(const int from, const int to) const {
        assert(forewardEdgeReferenceExists(from, to) == backwardEdgeReferenceExists(from, to));
        return forewardEdgeReferenceExists(from, to);
    }


    void removeEdges(const int node) {
        for (int other : nodes[node]) {
            nodes[other].erase(node);
        }
    }

    template<class F> 
    inline void forEachOutgoingEdgeTarget(int node, F f) const {
        forEachEdgeTarget(node, f);
    }
    template<class F> 
    inline void forEachIncomingEdgeSource(int node, F f) const {
        forEachEdgeTarget(node, f);
    }
    template<class F> 
    void forEachEdgeTarget(int node, F f) const {
        for (int to : nodes[node]) {
            f(to);
        }
    }

private:

    inline bool forewardEdgeReferenceExists(const int from, const int to) const {
        return nodes[from].find(to) != nodes[from].end();
    }
    inline bool backwardEdgeReferenceExists(const int from, const int to) const {
        return nodes[to].find(from) != nodes[to].end();
    }
};

}