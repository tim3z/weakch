#pragma once

#include <vector>
#include <unordered_set>
#include <assert.h>

namespace Contraction {

struct Node {
    std::unordered_set<int> in;
    std::unordered_set<int> out;
};

class Graph {
private: 
    std::vector<Node> nodes;

public:
    Graph(const int nodeCount = 0) : nodes(nodeCount) {}

    void setNodeCount(int count) {
        nodes.resize(count);
    }

    inline int nodeCount() const {
        return nodes.size();
    }

    int edgeCount() const {
        int count = 0;
        for (const Node &node : nodes) {
            count += node.out.size();
        }
        return count;
    }

    int edgeCount(const int node) const {
        return nodes[node].out.size();
    }

    int incomingEdgeCount(const int node) const {
        return nodes[node].in.size();
    }

    bool addEdge(const int from, const int to) {
        if (from != to && !edgeExists(from, to)) {
            nodes[from].out.insert(to);
            nodes[to].in.insert(from);
            return true;
        }
        return false;
    }

    bool edgeExists(const int from, const int to) const {
        assert(forewardEdgeReferenceExists(from, to) == backwardEdgeReferenceExists(from, to));
        return forewardEdgeReferenceExists(from, to);
    }

    void removeEdges(const int node) {
        for (int from : nodes[node].in) {
            nodes[from].out.erase(node);
        }
        for (int to : nodes[node].out) {
            nodes[to].in.erase(node);
        }
    }

    template<class F> 
    void forEachOutgoingEdgeTarget(int node, F f) const {
        for (int to : nodes[node].out) {
            f(to);
        }
    }
    template<class F> 
    void forEachIncomingEdgeSource(int node, F f) const {
        for (int from : nodes[node].in) {
            f(from);
        }
    }

private:

    inline bool forewardEdgeReferenceExists(const int from, const int to) const {
        return nodes[from].out.find(to) != nodes[from].out.end();
    }
    inline bool backwardEdgeReferenceExists(const int from, const int to) const {
        return nodes[to].in.find(from) != nodes[to].in.end();
    }
};

}