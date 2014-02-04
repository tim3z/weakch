#pragma once

#include <cstdlib>

namespace Util {
	int stringToInt(const std::string s) {
		return atoi(s.c_str());
	}
}