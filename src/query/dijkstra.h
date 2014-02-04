#pragma once

#include <vector>
#include <climits>
#include <iostream>

#include "Common/DataStructures/Container/KHeap.h"
#include "util/timer.h"

#include "graph.h"

namespace Query {

class Dijkstra {
	class Label {
	private:
		int distance = INT_MAX;
		int distanceTimestamp = -1;
	public:
		inline int getDistance(const Dijkstra * dijkstra) const {
			return distanceTimestamp == dijkstra->time ? distance : INT_MAX;
		}
		inline void setDistance(const Dijkstra * dijkstra, const int newDistance) {
			distance = newDistance;
			distanceTimestamp = dijkstra->time;
		}
	};
	
private:
	const Graph &graph;

	Container::KHeap<2, int> queue;
	std::vector<Label> labels;
	int time;

	std::ofstream &log;
	
public:
	
	Dijkstra(const Graph& graph, std::ofstream &log) : 
		graph(graph), queue(graph.nodeCount()), labels(graph.nodeCount()), time(0), log(log) {
		log << "initialization_time,dijkstra_time\n";
	}
	
	int shortestPath(const int from, const int to) {
		Util::Timer timer;

		time++;
		queue.Clear();
		queue.Update(from, 0);
		labels[from].setDistance(this, 0);

		log << timer.elapsedMilliseconds() << ",";
		timer.restart();

		while (!queue.Empty()) {
			int currentNode, currentDistance;
			queue.ExtractMin(currentNode, currentDistance);

			if (currentNode == to) break;
			
			graph.forEachOutgoingEdge(currentNode, [&] (int target, int weight, int) {
				int newDistance = currentDistance + weight;
				if (newDistance < labels[target].getDistance(this)) {
					labels[target].setDistance(this, newDistance);
					queue.Update(target, newDistance);
				}
			});
		}
		
		log << timer.elapsedMilliseconds() << "\n";

		return labels[to].getDistance(this);
	}
};

}