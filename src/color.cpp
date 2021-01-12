#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <vector>
#include <iostream>

#include "color.hpp"

namespace fractaldive {
	std::vector<uint32_t> makePalette() {
		srand(time(NULL));

		std::vector<uint32_t> palette;
		std::vector<uint32_t> v;
		for(size_t i = 0; i < 256; ++i) {
			v.push_back(PASTELLE[i]);
		}
		size_t idx = rand() % v.size();
		uint32_t first = v[idx];
		v.erase(v.begin() + idx);

		while(!v.empty()) {
			idx = rand() % v.size();
			uint32_t second = v[idx];
			v.erase(v.begin() + idx);
			double diff = std::fabs(double(first - second));
			double step = diff / 32.0;

			for(size_t j = 0; j < 32; ++j) {
				palette.push_back(j * step);
			}
			first = second;
		}
		return palette;
	}

} /* namespace fractaldive */
