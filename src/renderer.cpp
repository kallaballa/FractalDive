#include "renderer.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <algorithm>
#include <map>

#include "printer.hpp"
#include "util.hpp"
#ifndef _AMIGA
#include "digital_filters.hpp"
#endif

namespace fractaldive {

double filter(LowPassFilter& lfp, const double& toppix, const double& pix) {
	lfp.update(toppix);
	return lfp.update(pix);
}

inline fd_iter_count_t Renderer::getCurrentMaxIterations() const {
	return maxIterations_;
}

// Generate the fractal image
void Renderer::render() {
	if (ThreadPool::cores() > 1) {
		//use a thread pool to reduce thread start overhead
		ThreadPool& tpool = ThreadPool::getInstance();
		size_t tpsize = tpool.size();
		//slice the image into horizontal stripes
		fd_dim_t sliceHeight = std::floor(fd_float_t(config_.height_) / tpsize);
		fd_dim_t remainder = (config_.height_ % sliceHeight);
		for (size_t i = 0; i < tpsize + 1; ++i) {
			if (i == tpsize) {
				if(remainder > 0)
					sliceHeight = remainder;
				else
					break;
			}
			//start a worker thread
			tpool.enqueue([&](const size_t& i, const fd_dim_t& width, const fd_dim_t& sliceHeight) {
#ifndef _AMIGA
				LowPassFilter lpf(0.01, 2 * M_PI * 100000);
#endif

				fd_iter_count_t currentIt = getCurrentMaxIterations();
				fd_iter_count_t iterations = 0;
				fd_coord_t yoff = 0;

				for (fd_dim_t y = sliceHeight * i; y < (sliceHeight * i + sliceHeight); y++) {
					yoff = y * width;
					for (fd_dim_t x = 0; x < width; x++) {
						iterations = mandelbrot(x, y, currentIt);
						if(iterations < currentIt) {
							size_t pSize = palette_.size();
							if(pSize > 0) {
#ifndef _AMIGA
								imageData_[yoff + x] = filter(lpf, yoff > 0 ? imageData_[yoff - width + x] : 0, palette_[iterations % pSize]);
#else
								imageData_[yoff + x] = iterations % palette_.size();
#endif
							} else {
								imageData_[yoff + x] = filter(lpf, yoff > 0 ? imageData_[yoff - width + x] : 0, 0);
							}
						} else {
							imageData_[yoff + x] = filter(lpf, yoff > 0 ? imageData_[yoff - width + x] : 0, 0);
						}
					}
				}
			}, i, config_.width_, sliceHeight);
		}
	} else {
#ifndef _AMIGA
		LowPassFilter lpf(0.01, 2 * M_PI * 100000);
#endif
		fd_iter_count_t currentIt = getCurrentMaxIterations();
		fd_iter_count_t iterations = 0;
		fd_coord_t yoff = 0;

		for (fd_dim_t y = 0; y < config_.height_; y++) {
			yoff = y * config_.width_;
			for (fd_dim_t x = 0; x < config_.width_; x++) {
				iterations = mandelbrot(x, y, currentIt);
				if (iterations < currentIt) {
					size_t pSize = palette_.size();
					if(pSize > 0) {
#ifndef _AMIGA
						imageData_[yoff + x] = lpf.update(palette_[iterations % pSize]);
#else
						imageData_[yoff + x] = iterations % pSize;
#endif
					} else {
						imageData_[yoff + x] = 0;
					}
				} else {
					imageData_[yoff + x] = 0;
				}
			}
		}
	}
}

#if 0
// LUT experiments for AMIGA. doesn't make a real difference yet.
static std::vector<fd_mandelfloat_t> LUT(std::pow(2, 8),0);
inline fd_mandelfloat_t Renderer::square(const fd_mandelfloat_t& n) const {
	uint8_t idx = n.GetRawVal() >> 4;

	auto& p = LUT[idx];

	if(p == 0) {
		return p = n * n;
	} else {
		return p;
	}
}
#else
inline fd_mandelfloat_t Renderer::square(const fd_mandelfloat_t& n) const {
	return n * n;
}
#endif

inline fd_iter_count_t Renderer::mandelbrot(const fd_coord_t& x, const fd_coord_t& y, const fd_iter_count_t& currentIt) {
#if 1
	fd_iter_count_t iterations = 0;
	fd_mandelfloat_t x0 = (x + camera_.getOffsetX() + camera_.getPanX()) / (camera_.getZoom() / 10.0);
	fd_mandelfloat_t y0 = (y + camera_.getOffsetY() + camera_.getPanY()) / (camera_.getZoom() / 10.0);

	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t zrsqr = 0;
	fd_mandelfloat_t zisqr = 0;
	fd_quadfloat_t pointr = x0 / config_.width_; //0.0 - 1.0
	fd_quadfloat_t pointi = y0 / config_.height_; //0.0 - 1.0
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
