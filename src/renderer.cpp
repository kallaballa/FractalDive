#include "renderer.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <algorithm>

#include "printer.hpp"
#include "util.hpp"

namespace fractaldive {

fd_iter_count_t Renderer::getCurrentMaxIterations() const {
	return maxIterations_;
//	return std::min(fd_float_t(maxIterations_), (camera_.getZoomCount() / 3.0) + config_.startIterations_);
}
// Generate the fractal image
void Renderer::render() {
	if (ThreadPool::cores() > 1) {
		//use a thread pool to reduce thread start overhead
		ThreadPool& tpool = ThreadPool::getInstance();
		size_t tpsize = tpool.size();
		//slice the image into vertical stripes
		fd_dim_t sliceHeight = std::floor(fd_float_t(config_.height_) / tpsize);
		fd_dim_t remainder = (config_.height_ % sliceHeight);
		fd_dim_t extra = 0;
		for (size_t i = 0; i < tpsize; ++i) {
			if (i == (tpsize - 1) && remainder > 0)
				extra = remainder;

			//start a worker thread
			tpool.enqueue([&](const size_t& i, const fd_dim_t& width, const fd_dim_t& sliceHeight, const fd_dim_t& extra) {
				fd_iter_count_t currentIt = getCurrentMaxIterations();
				fd_iter_count_t iterations = 0;
				fd_coord_t yoff = 0;

				for (fd_dim_t y = 0; y < config_.height_; y++) {
					yoff = y * config_.width_;
					for (fd_dim_t x = 0; x < config_.width_; x++) {
						iterations = mandelbrot(x, y, currentIt);
						if(iterations < currentIt) {
#ifndef _AMIGA
							imageData_[yoff + x] = palette_[iterations % palette_.size()];
#else
							imageData_[yoff + x] = iterations % palette_.size();
#endif
						} else {
							imageData_[yoff + x] = 0;
						}
					}
				}
			}, i, config_.width_, sliceHeight, extra);
		}
		tpool.join();
	} else {
		fd_iter_count_t currentIt = getCurrentMaxIterations();
		fd_iter_count_t iterations = 0;
		fd_coord_t yoff = 0;

		for (fd_dim_t y = 0; y < config_.height_; y++) {
			yoff = y * config_.width_;
			for (fd_dim_t x = 0; x < config_.width_; x++) {
				iterations = mandelbrot(x, y, currentIt);
				if (iterations < currentIt) {
#ifndef _AMIGA
							imageData_[yoff + x] = palette_[iterations % palette_.size()];
#else
							imageData_[yoff + x] = iterations % palette_.size();
#endif
				} else {
					imageData_[yoff + x] = 0;
				}
			}
		}
	}

//	std::cout << getCurrentIterations() << std::endl;
}
inline fd_mandelfloat_t Renderer::square(const fd_mandelfloat_t& n) const {
	return n * n;
}

inline fd_iter_count_t Renderer::mandelbrot(const fd_coord_t& x, const fd_coord_t& y, const fd_iter_count_t& currentIt) {
#if 1
	fd_iter_count_t iterations = 0;
	fd_mandelfloat_t x0 = (x + camera_.getOffsetX() + camera_.getPanX()) / (camera_.getZoom() / 10.0);
	fd_mandelfloat_t y0 = (y + camera_.getOffsetY() + camera_.getPanY()) / (camera_.getZoom() / 10.0);

	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t zrsqr = 0;
	fd_mandelfloat_t zisqr = 0;
	fd_mandelfloat_t pointr = x0 / config_.width_; //0.0 - 1.0
	fd_mandelfloat_t pointi = y0 / config_.height_; //0.0 - 1.0
	fd_mandelfloat_t four = 4.0;

	//Algebraically optimized version that uses addition/subtraction as often as possible while reducing multiplications
	//and limiting multiplications to squaring only. this pretty nicely compiles to asm on Linux x86_64 (+simd), WASM (+simd) and m68k (000/020/030)
	//because types are chosen very carefully in "types.hpp"
	while (iterations < currentIt && zrsqr + zisqr <= four) {
		//zi = (square(zr + zi) - zrsqr) - zisqr; //equals line below as a consequence of binomial expansion
		zi = (zr + zr) * zi;
		zi += pointi;
		zr = (zrsqr - zisqr) + pointr;

		zrsqr = square(zr);
		zisqr = square(zi);

		++iterations;
	}
	return iterations;
#else
	float x0 = (x + camera_.getOffsetX() + camera_.getPanX()) / (camera_.getZoom() / 10.0);
	float y0 = (y + camera_.getOffsetY() + camera_.getPanY()) / (camera_.getZoom() / 10.0);
	std::complex<float> point(x0/config_.width_, y0/config_.height_);
	std::complex<float> z(0, 0);
	fd_iter_count_t iterations = 0;
	while (abs (z) < 2 && iterations < maxIterations_) {
		z = z * z + point;
		++iterations;
	}

	return iterations;
#endif
}


} /* namespace fractaldive */
