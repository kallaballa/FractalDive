#ifndef SRC_TILINGKERNEL_HPP_
#define SRC_TILINGKERNEL_HPP_

#include <cmath>
#include <vector>
#include "types.hpp"

namespace fractaldive {
template<size_t Tsize>
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
	const fd_coord_t size_ = Tsize;
	void initAt(const fd_coord_t& gravityX, const fd_coord_t& gravityY) {
		for (fd_coord_t y = 0; y < size_; ++y) {
			for (fd_coord_t x = 0; x < size_; ++x) {
				const size_t& calX = std::round((fd_float_t(x) / (size_ - 1.0)) * 4.0);
				const size_t& calY = std::round((fd_float_t(y) / (size_ - 1.0)) * 4.0);
				fd_float_t gravity = ((((size_ - std::abs(gravityX - x)) + (size_ - std::abs(gravityY - y))) / 2.0) / size_);
				kernel[x][y] = gravity * calibration[calY][calX];
			}
		}
	}

	fd_float_t* operator[](const size_t& x) {
		return kernel[x];
	}
};
}

#endif /* SRC_TILINGKERNEL_HPP_ */
