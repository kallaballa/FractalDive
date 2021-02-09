#ifndef SRC_TILINGKERNEL_HPP_
#define SRC_TILINGKERNEL_HPP_

#include <cmath>
#include <vector>
#include "types.hpp"

namespace fractaldive {
template<fd_coord_t Tsize>
class TilingKernel {
private:
	std::vector<std::vector<fd_float_t>> calibration = {
			{ 0.1, 0.3, 0.5, 0.7, 0.9 },
			{ 0.3, 0.4, 0.5, 0.6, 0.7 },
			{ 0.5, 0.5, 0.5, 0.5, 0.5 },
			{ 0.7, 0.6, 0.5, 0.4, 0.3 },
			{ 0.9, 0.7, 0.5, 0.3, 0.1 }, };
	fd_float_t kernel[Tsize][Tsize];
public:
	const size_t size_ = Tsize;
	void initAt(const fd_coord_t& gravityX, const fd_coord_t& gravityY) {
		for (fd_coord_t y = 0; y < Tsize; ++y) {
			for (fd_coord_t x = 0; x < Tsize; ++x) {
//				const size_t& calX = round((fd_float_t(x) / (Tsize - 1.0)) * 4.0);
//				const size_t& calY = round((fd_float_t(y) / (Tsize - 1.0)) * 4.0);
				fd_float_t gravity = ((((Tsize - std::abs(gravityX - x)) + (Tsize - std::abs(gravityY - y))) / 2.0) / Tsize);
				kernel[x][y] = gravity; //* calibration[calY][calX];
			}
		}
	}

	fd_float_t* operator[](const size_t& x) {
		return kernel[x];
	}
};
}

#endif /* SRC_TILINGKERNEL_HPP_ */
