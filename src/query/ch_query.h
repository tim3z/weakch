#pragma once

#include <vector>
#include <climits>
#include <iostream>
#include <assert.h>

#include "Common/DataStructures/Container/KHeap.h"
#include "util/timer.h"

#include "graph.h"

namespace Query {

class CHQuery {
	class Label {
	private:
		int distance = INT_MAX;
		int distanceTimestamp = -1;
	public:
		inline int getDistance(const CHQuery * dijkstra) const {
			return distanceTimestamp == dijkstra->time ? distance : INT_MAX;
		}
		inline void setDistance(const CHQuery * dijkstra, const int newDistance) {
			distance = newDistance;
			distanceTimestamp = dijkstra->time;
		}
	};
	
private:
	const Graph &graph;

	Container::KHeap<2, int> forwardQueue;
	Container::KHeap<2, int> backwardQueue;
	std::vector<Label> forwardLabels;
	std::vector<Label> backwardLabels;
	int time;

	std::ofstream &log;
	
public:
	
	CHQuery(const Graph& graph, std::ofstream &log) : 
		graph(graph), forwardQueue(graph.nodeCount()), backwardQueue(graph.nodeCount()), forwardLabels(graph.nodeCount()), backwardLabels(graph.nodeCount()), time(0), log(log) {

		log << "initialization_time,dijkstra_time\n";
	}
	
	int shortestPath(const int from, const int to) {
		Util::Timer timer;

		time++;
		forwardQueue.Clear();
		backwardQueue.Clear();
		forwardQueue.Update(from, 0);
		backwardQueue.Update(to, 0);
		forwardLabels[from].setDistance(this, 0);
		backwardLabels[to].setDistance(this, 0);
		int tentativeDistance = INT_MAX;

		log << timer.elapsedMilliseconds() << ",";
		timer.restart();

		bool forwardDone = false;
		bool backwardDone = false;
		while (!forwardDone || !backwardDone) {
			int currentNode, currentDistance;
			int otherDistance;
			// FORWARD
			forwardDone = forwardDone || forwardQueue.Empty();
			if (!forwardDone) {
				forwardQueue.ExtractMin(currentNode, currentDistance);
				assert(currentDistance == forwardLabels[currentNode].getDistance(this));
				assert(currentDistance != INT_MAX);

				otherDistance = backwardLabels[currentNode].getDistance(this);
				if (otherDistance != INT_MAX) {
					int newTentativeDistance = currentDistance + otherDistance; 
					if (newTentativeDistance < tentativeDistance) {
						tentativeDistance = newTentativeDistance;
					}
				}
				if (currentDistance >= tentativeDistance) forwardDone = true;
				
				graph.forEachOutgoingEdge(currentNode, [&] (int target, int weight, int) {
					int newDistance = currentDistance + weight;
					if (newDistance < forwardLabels[target].getDistance(this)) {
						forwardLabels[target].setDistance(this, newDistance);
						forwardQueue.Update(target, newDistance);
					}
				});
			}

			// BACKWARD
			backwardDone = backwardDone || backwardQueue.Empty();
			if (!backwardDone) {
				backwardQueue.ExtractMin(currentNode, currentDistance);
				assert(currentDistance == backwardLabels[currentNode].getDistance(this));
				assert(currentDistance != INT_MAX);

				otherDistance = forwardLabels[currentNode].getDistance(this);
				if (otherDistance != INT_MAX) {
					int newTentativeDistance = currentDistance + otherDistance;
					if (newTentativeDistance < tentativeDistance) {
						tentativeDistance = newTentativeDistance;
					}
				}
				if (currentDistance >= tentativeDistance) backwardDone = true;
				
				graph.forEachIncomingEdge(currentNode, [&] (int target, int weight, int) {
					int newDistance = currentDistance + weight;
					if (newDistance < backwardLabels[target].getDistance(this)) {
						backwardLabels[target].setDistance(this, newDistance);
						backwardQueue.Update(target, newDistance);
					}
				});
			}
		}
		
		log << timer.elapsedMilliseconds() << "\n";

		return tentativeDistance;
	}
};

}