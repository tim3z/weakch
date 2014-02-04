#pragma once

#include <vector>
#include <algorithm>

namespace VectorUtils {
    template<class T>
    inline typename std::vector<T>::const_iterator find(const std::vector<T> &v, const T &value) {
        return std::find(v.begin(), v.end(), value);
    }

    template<class T> 
    inline void remove(std::vector<T> &v, const T &toRemove) {
        v.erase(std::remove(v.begin(), v.end(), toRemove), v.end());
    }

    template<class T, class F>
    void sort(std::vector<T> &v, F f) {
    	for (unsigned int i = 0; i < v.size(); i++) {
			for (unsigned int j = i; j < v.size() - 1; j++) {
				if (f(v[j], v[j+1])) {
					T tmp = v[j];
					v[j] = v[j+1];
					v[j+1] = tmp;
				}
			}
		}
    }
}