#include "renderer.hpp"

#include <cassert>
#include <cmath>

#include "threadpool.hpp"
#include "printer.hpp"
#include "timetracker.hpp"
#include "util.hpp"


namespace fractaldive {

// Generate the fractal image
void Renderer::render(const bool& shadowOnly) {
	TimeTracker& tt = TimeTracker::getInstance();

	tt.execute("dive.render", [&](){
		if (ThreadPool::cores() > 1) {
			//use a thread pool to reduce thread start overhead
			ThreadPool& tpool = ThreadPool::getInstance();
			size_t tpsize = tpool.size();
			//slice the image into vertical stripes
			fd_dim_t sliceHeight = std::floor(fd_float_t(HEIGHT_) / tpsize);
			fd_dim_t remainder = (HEIGHT_ % sliceHeight);
			fd_dim_t extra = 0;
			for (size_t i = 0; i < tpsize; ++i) {
				if (i == (tpsize - 1) && remainder > 0)
					extra = remainder;

				//start a worker thread
				tpool.enqueue([&](const size_t& i, const fd_dim_t& width, const fd_dim_t& sliceHeight, const fd_dim_t& extra, const bool& shadowOnly) {
					TimeTracker& tt = TimeTracker::getInstance();

					fd_iter_count_t iterations = 0;
					size_t index = 0;
					for (fd_dim_t y = sliceHeight * i; y < (sliceHeight * (i + 1)) + extra; y++) {
						for (fd_dim_t x = 0; x < width; x++) {
							tt.execute("dive.render.mandelbrot", [&](){
								iterations = mandelbrot(x, y);
							});
							tt.execute("dive.render.calcIndex", [&](){
								index = calculatePaletteIndex(iterations, getMaxIterations());
							});

							tt.execute("dive.render.colorPixel", [&](){
								colorPixelAt(x, y, index, iterations, shadowOnly);
							});
						}
					}
				}, i, WIDTH_, sliceHeight, extra, shadowOnly);
			}
		} else {
			fd_iter_count_t iterations = 0;
			size_t index = 0;
			for (fd_dim_t y = 0; y < HEIGHT_; y++) {
				for (fd_dim_t x = 0; x < WIDTH_; x++) {
					tt.execute("dive.render.mandelbrot", [&](){
						iterations = mandelbrot(x, y);
					});

					tt.execute("dive.render.calcIndex", [&](){
						index = calculatePaletteIndex(iterations, maxIterations_);
					});

					tt.execute("dive.render.colorPixel", [&](){
						colorPixelAt(x, y, index, iterations, shadowOnly);
					});
				}
			}
		}
	});
}

inline fd_mandelfloat_t Renderer::square(const fd_mandelfloat_t& n) const {
	return n * n;
}

inline fd_iter_count_t Renderer::mandelbrot(const fd_coord_t& x, const fd_coord_t& y) const  {
#if 1
	fd_iter_count_t iterations = 0;
	fd_mandelfloat_t x0 = (x + offsetx_ + panx_) / (zoom_ / 10.0);
	fd_mandelfloat_t y0 = (y + offsety_ + pany_) / (zoom_ / 10.0);

	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t zrsqr = 0;
	fd_mandelfloat_t zisqr = 0;
	fd_mandelfloat_t pointr = x0 / WIDTH_; //0.0 - 1.0
	fd_mandelfloat_t pointi = y0 / HEIGHT_; //0.0 - 1.0
	fd_mandelfloat_t four = 4.0;

	//Algebraically optimized version that uses addition/subtraction as often as possible while reducing multiplications
	//and limiting multiplications to squaring only. this pretty nicely compiles to asm on Linux x86_64 (+simd), WASM (+simd) and m68k (000/020/030)
	//because types are chosen very carefully in "types.hpp"
	while (iterations < maxIterations_ && zrsqr + zisqr <= four) {
		//zi = (square(zr + zi) - zrsqr) - zisqr; //equals line below as a consequence of binomial expansion
		zi = (zr + zr) * zi;
		zi += pointi;
		zr = (zrsqr - zisqr) + pointr;

		zrsqr = square(zr);
		zisqr = square(zi);

		++iterations;
	}
#else
	float x0 = (x + offsetx_ + panx_) / (zoom_ / 10);
	float y0 = (y + offsety_ + pany_) / (zoom_ / 10);
	std::complex<float> point(x0/WIDTH_, y0/HEIGHT_);
	std::complex<float> z(0, 0);
	fd_iter_count_t iterations = 0;
	while (abs (z) < 2 && iterations < maxIterations_) {
	    z = z * z + point;
	    ++iterations;
	}

	return iterations;
#endif
	return iterations;
}

size_t Renderer::calculatePaletteIndex(const fd_iter_count_t& iterations,	const fd_iter_count_t& maxIterations) const {
	size_t index = 0;
#ifndef _NOSHADOW
	if (iterations < maxIterations) {
#ifndef _FIXEDPOINT
		// 254 so we can use 0 as index for black
		index = fd_mandelfloat_t(iterations) / maxIterations * 254 + 1;
#else
		// 254.0 so we can use 0 as index for black
		index = (iterations / maxIterations_ * 254.0).ToInt<uint8_t>() + 1;
#endif
	}
#endif
	return index;
}

/*
 * FIXME: Thread safety
 */
void Renderer::colorPixelAt(const fd_coord_t& x, const fd_coord_t& y, const size_t& index, const fd_iter_count_t& iterations, const bool& shadowOnly) {
#ifndef _NO_SHADOW
		fd_image_pix_t color;
		get_color_from_palette(color, index);
		size_t pixelindex = (y * WIDTH_ + x);

		// Apply the argb color
		if (!shadowOnly) {
			imgdata_[pixelindex] = color;
		}

		//cheap greyscale. we don't need perceptual acuity to measure detail.
		assert(index < 256);
		shadowData_[pixelindex] = index;
#else
		size_t pixelindex = (y * WIDTH_ + x);
		// Apply the color
			if (index == 0) {
				imgdata_[pixelindex] = 0;
			} else {
	#ifndef _FIXEDPOINT
				imgdata_[pixelindex] = (iterations / maxIterations_) * std::numeric_limits<fd_image_pix_t>::max();
	#else
				imgdata_[pixelindex] = iterations.GetRawVal();
	#endif
			}
#endif
}


// Zoom the fractal
void Renderer::zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin) {
	std::cerr << "F:" << factor << std::endl;
	if (zoomin) {
		// Zoom in
		zoom_ *= factor;
		panx_ = factor * (x + offsetx_ + panx_);
		pany_ = factor * (y + offsety_ + pany_);
	} else {
		// Zoom out
		zoom_ /= factor;
		panx_ = (x + offsetx_ + panx_) / factor;
		pany_ = (y + offsety_ + pany_) / factor;
	}
}

void Renderer::resetSmoothPan() {
	panHistoryX_.clear();
	panHistoryY_.clear();
}

void Renderer::initSmoothPan(const fd_coord_t& x, const fd_coord_t& y) {
	panHistoryX_.resize(panSmoothLen_);
	panHistoryY_.resize(panSmoothLen_);

	fd_float_t xstep = (x / panSmoothLen_);
	fd_float_t ystep = (y / panSmoothLen_);

	for(size_t i = 0; i < panSmoothLen_; ++i) {
		panHistoryX_[i] = (xstep * i);
	}

	for(size_t i = 0; i < panSmoothLen_; ++i) {
		panHistoryY_[i] = (ystep * i);
	}
}

std::pair<fd_coord_t, fd_coord_t> Renderer::smoothPan(const fd_coord_t& x, const fd_coord_t& y) {
	assert((panHistoryX_.empty() && panHistoryY_.empty()) || (!panHistoryX_.empty() && !panHistoryY_.empty()));
	if(panHistoryX_.empty()) {
		initSmoothPan(x,y);
	}

	panHistoryX_.pop_back();
	panHistoryY_.pop_back();
	panHistoryX_.push_front(x);
	panHistoryY_.push_front(y);

	fd_coord_t xhtotal = 0;

	for(const auto& xh : panHistoryX_) {
		xhtotal += xh;
	}

	fd_coord_t yhtotal = 0;
	for(const auto& yh : panHistoryY_) {
		yhtotal += yh;
	}

	return {fd_float_t(xhtotal) / panSmoothLen_, fd_float_t(yhtotal) / panSmoothLen_};
}

// Pan the fractal
void Renderer::pan(const fd_coord_t& x, const fd_coord_t& y) {
	std::cerr << "P:" << x << " \t" << y << std::endl;

	auto ft = smoothPan(x,y);
	panx_ += ft.first;
	pany_ += ft.second;
}
} /* namespace fractaldive */
