#pragma once

#include <stdlib.h>
#include <time.h>

namespace Util {
class Random {
public:
	Random(unsigned int seed = time(NULL)) {
		#ifdef VERBOSE
			std::cout << "seed: " << seed << "\n";
		#endif
		srand(seed);
	}

	int getRandomInt(int max) {
		return rand() % max;
	}
};
}