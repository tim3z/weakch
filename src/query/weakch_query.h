#pragma once

#include <vector>
#include <climits>
#include <iostream>
#include <assert.h>

#include "Common/DataStructures/Container/KHeap.h"
#include "util/timer.h"

#include "graph.h"

#define SET_FOREWARD_DISTANCE(node, distance) forwardLabels[node].setDistance(this, distance)
#define SET_BACKWARD_DISTANCE(node, distance) backwardLabels[node].setDistance(this, distance)
#define GET_DISTANCE(container, node) container[node].getDistance(this)

namespace Query {

class WeakCHQuery {
	class Label {
	private:
		int distance = INT_MAX;
		int distanceTimestamp = -1;
	public:
		inline int getDistance(const WeakCHQuery * dijkstra) const {
			return distanceTimestamp == dijkstra->time ? distance : INT_MAX;
		}
		inline void setDistance(const WeakCHQuery * dijkstra, const int newDistance) {
			distance = newDistance;
			distanceTimestamp = dijkstra->time;
		}
	};
	
private:
	const Graph &graph;
	const std::vector<int> &eliminationTreeData;

	Container::KHeap<2, int> forwardQueue;
	Container::KHeap<2, int> backwardQueue;
	std::vector<Label> forwardLabels;
	std::vector<Label> backwardLabels;
	std::vector<bool> inSearchSpace;
	std::vector<int> inSeachSpaceIds;
	int time;

	std::ofstream &log;
	
public:
	
	WeakCHQuery(const Graph& graph, const std::vector<int> &eliminationTreeData, std::ofstream &log, const int uncontracted = 0) : 
		graph(graph), 
		eliminationTreeData(eliminationTreeData), 
		forwardQueue(graph.nodeCount()), 
		backwardQueue(graph.nodeCount()), 
		forwardLabels(graph.nodeCount()), 
		backwardLabels(graph.nodeCount()), 
		inSearchSpace(graph.nodeCount(), false), 
		time(0), 
		log(log) {

		for (int i = graph.nodeCount() - uncontracted; i < graph.nodeCount(); i++) {
			inSearchSpace[i] = true;
		}

		log << "initialization_time,dijkstra_time\n";
	}
	
	int shortestPath(const int from, const int to) {
		Util::Timer timer;

		time++;
		forwardQueue.Clear();
		backwardQueue.Clear();
		forwardQueue.Update(from, 0);
		backwardQueue.Update(to, 0);
		inSeachSpaceIds.clear();
		aquireSearchSpace(from, to);
		SET_FOREWARD_DISTANCE(from, 0);
		SET_BACKWARD_DISTANCE(to, 0);
		int tentativeDistance = INT_MAX;
		int lastDistance = 0;

		log << timer.elapsedMilliseconds() << ",";
		timer.restart();

		while (!forwardQueue.Empty() || !backwardQueue.Empty()) {
			int currentNode, currentDistance;
			int otherDistance;
			// FORWARD
			if (!forwardQueue.Empty()) {
				forwardQueue.ExtractMin(currentNode, currentDistance);
				assert(currentDistance == GET_DISTANCE(forwardLabels, currentNode));
				assert(currentDistance != INT_MAX);

				otherDistance = GET_DISTANCE(backwardLabels, currentNode);
				if (otherDistance != INT_MAX) {
					int newTentativeDistance = currentDistance + otherDistance; 
					if (newTentativeDistance < tentativeDistance) {
						tentativeDistance = newTentativeDistance;
					}
				}
				if (currentDistance + lastDistance >= tentativeDistance) break;
				lastDistance = currentDistance;
				
				graph.forEachOutgoingEdge(currentNode, inSearchSpace, [&] (int target, int weight, int) {
					assert(weight != INT_MAX);
					if (inSearchSpace[target]) {
						int newDistance = currentDistance + weight;
						if (newDistance < GET_DISTANCE(forwardLabels, target)) {
							SET_FOREWARD_DISTANCE(target, newDistance);
							forwardQueue.Update(target, newDistance);
						}
					}
				});
			}

			// BACKWARD
			if (!backwardQueue.Empty()) {
				backwardQueue.ExtractMin(currentNode, currentDistance);
				assert(currentDistance == GET_DISTANCE(backwardLabels, currentNode));
				assert(currentDistance != INT_MAX);

				otherDistance = GET_DISTANCE(forwardLabels, currentNode);
				if (otherDistance != INT_MAX) {
					int newTentativeDistance = currentDistance + otherDistance;
					if (newTentativeDistance < tentativeDistance) {
						tentativeDistance = newTentativeDistance;
					}
				}
				if (currentDistance + lastDistance >= tentativeDistance) break;
				lastDistance = currentDistance;
				
				graph.forEachIncomingEdge(currentNode, inSearchSpace, [&] (int target, int weight, int) {
					assert(weight != INT_MAX);
					if (inSearchSpace[target]) {
						int newDistance = currentDistance + weight;
						if (newDistance < GET_DISTANCE(backwardLabels, target)) {
							SET_BACKWARD_DISTANCE(target, newDistance);
							backwardQueue.Update(target, newDistance);
						}
					}
				});
			}
		}

		for (int node : inSeachSpaceIds) {
			inSearchSpace[node] = false;
		}

		log << timer.elapsedMilliseconds() << "\n";

		return tentativeDistance;
	}

private:

	void aquireSearchSpace(const int from, const int to) {
		int currentNode = from;
		while(currentNode != -1 && !inSearchSpace[currentNode]) {
			inSearchSpace[currentNode] = true;
			inSeachSpaceIds.push_back(currentNode);
			currentNode = eliminationTreeData[currentNode];
		}

		currentNode = to;
		while(currentNode != -1 && !inSearchSpace[currentNode]) {
			inSearchSpace[currentNode] = true;
			inSeachSpaceIds.push_back(currentNode);
			currentNode = eliminationTreeData[currentNode];
		}
	}
};

}