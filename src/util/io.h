#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept> 

#include "cast.h"
#include "split.h"

namespace IOUtils {

void readPermutation(const std::string filename, std::vector<int> &permutation) {
    std::cout << "reading permutation " << filename << "\n";

    std::ifstream f(filename);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    int nodeCount = 0;
    int rank = -1;
    while(f >> rank) {
        permutation[rank] = nodeCount++; 
    }

    std::cout << "done\n";
}

void readRanks(const std::string filename, std::vector<int> &ranks) {
    std::cout << "reading ranks " << filename << "\n";

    std::ifstream f(filename);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    int node = 0;
    int rank = -1;
    while(f >> rank) {
        ranks[node++] = rank;
    }

    std::cout << "done\n";
}

template<class F1, class F2>
void readGraph(std::string filename, F1 graphSizeCallback, F2 edgeCallback) {
    readFile(filename, [&] (std::string &line) {
        if (line == "") return;

        // Get the problem size
        if (line[0] == 'p') {
            std::vector<std::string> tokens = Util::split(line, ' ');
            if (tokens.size() != 4) {
                throw std::runtime_error("file broken");
            }

            int numVertices = Util::stringToInt(tokens[2]);
            int numEdges = Util::stringToInt(tokens[3]);
            std::cout << numVertices << " nodes, " << numEdges << " edges expected ... \n";
            graphSizeCallback(numVertices, numEdges);
        }

        if (line[0] == 'a') {
            std::vector<std::string> tokens = Util::split(line, ' ');
            std::vector<int> args;
            for (unsigned int i = 1; i < tokens.size(); i++) {
                args.push_back(Util::stringToInt(tokens[i]));
            }
            edgeCallback(args);
        }
    });
}

template<class F>
void openFile(std::string filename, F callback, std::ios_base::openmode mode = std::ios::in) {
    std::cout << "Opening file " << filename << "\n";
    std::ifstream f(filename, mode);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    callback(f);

    std::cout << "done reading " << filename << "\n";
}

template<class F>
void readFile(std::string filename, F lineCallback) {
    std::cout << "Reading file " << filename << "\n";
    std::ifstream f(filename);
    if(!f.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    std::string line;
    while (getline(f, line)) {
        lineCallback(line);
    }

    std::cout << "done reading " << filename << "\n";
}

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

template<class Type>
inline void writeElementary(std::ofstream &file, const Type &object) {
    file.write(reinterpret_cast<const char*>(&object), sizeof(object));
}

template<class Type>
inline void readElementary(std::ifstream &file, Type &object) {
    file.read(reinterpret_cast<char*>(&object), sizeof(object));
}

template<class Type>
void writeVector(std::ofstream &file, std::vector<Type> &vector) {
    unsigned int size = vector.size();
    writeElementary(file, size);
    for (Type &object : vector) {
        writeElementary(file, object);
    }
}

template<class Type>
void readVector(std::ifstream &file, std::vector<Type> &vector) {
    unsigned int size;
    readElementary(file, size);
    vector.resize(size);
    for (Type &object : vector) {
        readElementary(file, object);
    }
}

template<class Type, class F>
void writeVector(std::ofstream &file, std::vector<Type> &vector, F f) {
    unsigned int size = vector.size();
    writeElementary(file, size);
    for (Type &object : vector) {
        f(file, object);
    }
}

template<class Type, class F>
void readVector(std::ifstream &file, std::vector<Type> &vector, F f) {
    unsigned int size;
    readElementary(file, size);
    vector.resize(size);
    for (Type &object : vector) {
        f(file, object);
    }
}

}