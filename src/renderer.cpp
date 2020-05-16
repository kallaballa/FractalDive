#include "renderer.hpp"

#include <cassert>
#include <cmath>

#include "threadpool.hpp"

namespace fractaldive {
// Generate the fractal image
void Renderer::render(bool greyonly) {
	if(ThreadPool::extra_cores() > 0) {
  	ThreadPool& tpool = ThreadPool::getInstance();
  	size_t tpsize = tpool.size();
		// Iterate over the pixels
		fd_dim_t sliceHeight = std::floor(fd_float_t(HEIGHT_) / tpsize);
		fd_dim_t remainder = (HEIGHT_ % sliceHeight);
		fd_dim_t extra = 0;
		for (size_t i = 0; i < tpsize; ++i) {
			if (i == (tpsize - 1) && remainder > 0)
				extra = remainder;

			tpool.work([=]() {
				for (fd_dim_t y = sliceHeight * i; y < (sliceHeight * (i + 1)) + extra; y++) {
					for (fd_dim_t x = 0; x < WIDTH_; x++) {
						iterate(x, y, maxIterations_, greyonly);
					}
				}
			});
		}
		tpool.join();
	} else {
		for (fd_dim_t y = 0; y < HEIGHT_; y++) {
			for (fd_dim_t x = 0; x < WIDTH_; x++) {
				iterate(x, y, maxIterations_, greyonly);
			}
		}
	}
}

// Calculate the color of a specific pixel
void Renderer::iterate(const fd_coord_t& x, const fd_coord_t& y, const uint64_t& maxiterations, const bool& greyonly) {
	fd_mandelfloat_t xViewport = (x + offsetx_ + panx_) / (zoom_ / 10);
	fd_mandelfloat_t yViewport = (y + offsety_ + pany_) / (zoom_ / 10);
	uint64_t iterations = 0;
	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t cr = xViewport/WIDTH_;
	fd_mandelfloat_t ci = yViewport/HEIGHT_;
	fd_mandelfloat_t two = 2.0;
	fd_mandelfloat_t four = 4.0;
    while (iterations < maxIterations_ && zr * zr + zi * zi < four) {
    	const fd_mandelfloat_t& temp = zr * zr - zi * zi + cr;
      zi = two * zr * zi + ci;
      zr = temp;
      ++iterations;
    }
    color24_t color;
  	size_t index = 0;
  	if (iterations == maxiterations) {
  		color = {0, 0, 0}; // Black
  	} else {
  		// 254 so we can use 0 as index for black
  		index = std::floor((fd_float_t(iterations) / (maxiterations - 1)) * 254.0);
  		color = PALETTE[index];
  	}

  	size_t pixelindex = (y * WIDTH_ + x);
  	// Apply the color
  	if (!greyonly) {
  			rgbdata_[pixelindex] = color;
  	}

  	//cheap greyscale. we don't need perceptual acuity to measure detail.
  	if (iterations == maxiterations) {
  		greydata_[pixelindex] = 0;
  	} else {
  		assert(index < 256);
  		greydata_[pixelindex] = index + 1;
  	}
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

void Renderer::pan(const fd_coord_t& x, const fd_coord_t& y) {
	panx_ += x;
	pany_ += y;
}
} /* namespace fractaldive */
