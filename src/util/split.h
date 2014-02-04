#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace Util {
	// Implements string split using standard library alone.
	// Taken from http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	void split(const std::string s, const char delimiter, std::vector<std::string> &elements) {
	    std::stringstream ss(s);
	    std::string item;
	    while(getline(ss, item, delimiter)) {
	    	elements.push_back(item);
	    }
	}
	std::vector<std::string> split(const std::string s, const char delimiter) {
	    std::vector<std::string> elements;
	    split(s, delimiter, elements);
	    return elements;
	}
}
