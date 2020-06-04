#include "renderer.hpp"

#include <cassert>
#include <cmath>

#include "threadpool.hpp"
#include "printer.hpp"
#include "timetracker.hpp"


namespace fractaldive {
// Generate the fractal image
void Renderer::render(const bool& shadowonly) {
	TimeTracker::getInstance().execute("render", "mandelbrot", [&](){
		if (ThreadPool::cores() > 1) {
			//use a thread pool to reduce thread start overhead
			ThreadPool& tpool = ThreadPool::getInstance();
			size_t tpsize = tpool.size();
			//slice the image into vertical stripes
			fd_dim_t sliceHeight = std::floor(fd_float_t(HEIGHT_) / tpsize);
			fd_dim_t remainder = (HEIGHT_ % sliceHeight);
			fd_dim_t extra = 0;
			std::vector<std::future<void>> futures;
			for (size_t i = 0; i < tpsize; ++i) {
				if (i == (tpsize - 1) && remainder > 0)
					extra = remainder;

				//start a worker thread
				futures.push_back(tpool.enqueue([&](const size_t& i, const fd_dim_t& width, const fd_dim_t& sliceHeight, const fd_dim_t& extra, const bool& shadowonly) {
					for (fd_dim_t y = sliceHeight * i; y < (sliceHeight * (i + 1)) + extra; y++) {
						for (fd_dim_t x = 0; x < width; x++) {
							iterate(x, y, shadowonly);
						}
					}
				}, i, WIDTH_, sliceHeight, extra, shadowonly));
			}
			//FIXME: waiting for all worker threads is not working correctly
			for(auto& f : futures) {
				f.get();
			}
		} else {
			for (fd_dim_t y = 0; y < HEIGHT_; y++) {
				for (fd_dim_t x = 0; x < WIDTH_; x++) {
					iterate(x, y, shadowonly);
				}
			}
		}
	});
}

inline fd_mandelfloat_t Renderer::square(const fd_mandelfloat_t& n) const {
	return n * n;
}

inline fd_iter_count_t Renderer::mandelbrot(const fd_coord_t& x, const fd_coord_t& y) const {
	fd_iter_count_t iterations = 0;
	fd_mandelfloat_t xViewport = (x + offsetx_ + panx_) / (zoom_ / 10);
	fd_mandelfloat_t yViewport = (y + offsety_ + pany_) / (zoom_ / 10);

	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t zrsqr = 0;
	fd_mandelfloat_t zisqr = 0;
	fd_mandelfloat_t cr = xViewport / WIDTH_;
	fd_mandelfloat_t ci = yViewport / HEIGHT_;
	fd_mandelfloat_t four = 4.0;

	//Algebraically optimized version that uses addition/subtraction as often as possible while reducing multiplications
	//and limiting multiplications to squaring only. this pretty nicely compiles to asm on Linux x86_64 (+simd), WASM (+simd) and m68k (000/020/030)
	//because types are chosen very carefully in "types.hpp"
	while (iterations < maxIterations_ && zrsqr + zisqr <= four) {
		zi = square(zr + zi) - zrsqr - zisqr;
		zi += ci;
		zr = zrsqr - zisqr + cr;
		zrsqr = square(zr);
		zisqr = square(zi);
		iterations+=1;
	}

	return iterations;
}

void Renderer::colorPixelByPalette(const fd_coord_t& x, const fd_coord_t& y, const fd_iter_count_t iterations, const bool& shadowonly) {
#ifndef _NO_SHADOW
		fd_image_comp_t color(0,0,0);
		size_t index = 0;
		if (iterations != maxIterations_) {
	#ifndef _FIXEDPOINT
			// 254 so we can use 0 as index for black
			index = iterations / maxIterations_ * 254;
	#else
			// 254.0 so we can use 0 as index for black
			index = (iterations / maxIterations_ * 254.0).ToInt<uint8_t>();
	#endif
			if (!shadowonly) {
				color = PALETTE[index];
			}
		}

		size_t pixelindex = (y * WIDTH_ + x);
		// Apply the color
		if (!shadowonly) {
			imgdata_[pixelindex] = color;
		}
		std::numeric_limits<fd_image_comp_t>::max();
		//cheap greyscale. we don't need perceptual acuity to measure detail.
		if (iterations == maxIterations_) {
			shadowdata_[pixelindex] = 0;
		} else {
			assert(index < 256);
			shadowdata_[pixelindex] = index;
		}
#else
		size_t pixelindex = (y * WIDTH_ + x);
		// Apply the color
		if (!shadowonly) {
			if (iterations == maxIterations_) {
				imgdata_[pixelindex] = 0;
			} else {
	#ifndef _FIXEDPOINT
				imgdata_[pixelindex] = (iterations / maxIterations_) * std::numeric_limits<fd_image_comp_t>::max();
	#else
				imgdata_[pixelindex] = iterations.GetRawVal();
	#endif
			}
		}
#endif
}

// Calculate the color of a specific pixel. first find out how many iterations (<= maxIterations_) it takes and than color the pixel according to some kind of palette.
void Renderer::iterate(const fd_coord_t& x, const fd_coord_t& y, const bool& shadowonly) {
	TimeTracker& tt = TimeTracker::getInstance();

	fd_iter_count_t iterations = 0;
	tt.execute("render", "mandelbrot", [&](){
	 iterations = mandelbrot(x, y);
	});

	tt.execute("render", "coloring", [&](){
		colorPixelByPalette(x,y,iterations,shadowonly);
	});
}

// Zoom the fractal
void Renderer::zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin) {
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
// Pan the fractal
void Renderer::pan(const fd_coord_t& x, const fd_coord_t& y) {
	panx_ += x;
	pany_ += y;
}
} /* namespace fractaldive */
