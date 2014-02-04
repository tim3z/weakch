#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept> 

#include "Common/Assert.h"
#include "StreetData/Dimacs/DimacsBuildNetwork.h"
#include "Common/DataStructures/RoadNetwork/RoadNetwork.h"
#include "Common/DataStructures/Graph/Graphs.h"
#include "Common/Algorithms/CH/CH.h"
#include "Common/Tools/Timer.h"

template<class F>
void writeFile(std::string filename, F writeCallback, std::ios_base::openmode mode = std::ios::out) {
    std::cout << "Opening file " << filename << " (to write) \n";
    std::ofstream f(filename, mode);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    writeCallback(f);
    std::cout << "closing file " << filename << "\n";
}


/**
 * Performs a "traditional" contraction as implemented in the framework and obtains the order for further experiments
 */ 
int main(int argc, char **argv) {
	RoadNetwork::RoadNetwork<Graph::RoadShortcutGraph> network;
	Geometry::LatLongRectangle boundingBox;
	StreetData::DimacsBuildNetwork(network, argv[1], boundingBox, false, true);
	Graph::RoadShortcutGraph &graph = network.Graph();

	Algorithms::CH<Graph::RoadShortcutGraph> ch(network.Graph());
	Common::Tools::Timer timer;
	ch.Run(true);
	std::cout << "######################################################\n" << "contraction took: " << timer.ElapsedMilliseconds() << "ms\n";
	std::vector<int> ranks = ch.VertexRanks();

	writeFile("traditional_ch_ranks", [&] (std::ofstream &file) {
		for (int rank : ranks) {
			file << rank << "\n";
		}
	});

	writeFile("traditional_ch_result.gr", [&] (std::ofstream &file) {
		file << "p sp " << network.Graph().NumVertices() << " " << network.Graph().NumEdges() << "\n";
		FORALL_EDGES(network.Graph(), node, edge) {
			if (edge->Forward()) {
				file << "a " << node + 1 << " " << edge->OtherVertexIndex() + 1 << " " << edge->Weight() << " " << edge->MiddleVertexIndex() << "\n";
			} else {
				file << "a " << edge->OtherVertexIndex() + 1 << " " << node + 1 << " " << edge->Weight() << " " << edge->MiddleVertexIndex() << "\n";
			}
		}
	});
}
