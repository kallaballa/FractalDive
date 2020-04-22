// ------------------------------------------------------------------------
// Based on:
// http://rembound.com/articles/drawing-mandelbrot-fractals-with-html5-canvas-and-javascript
// ------------------------------------------------------------------------

#include "renderer.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <thread>
#include <iostream>

namespace fractaldive {

// Generate palette
void Renderer::generatePalette() {
	size_t increment = palette24_t::SIZE_ / 256;
	for (size_t i = 0; i < 256; i++) {
		palette_[i] = palette24_t::ITEMS_[i * increment];
	}
}

// Generate the fractal image
void Renderer::render(bool greyonly) {
	// Iterate over the pixels
#ifndef _JAVASCRIPT_MT
	for (dim_t y = 0; y < HEIGHT_; y++) {
		#pragma omp parallel for ordered schedule(dynamic)
		for (dim_t x = 0; x < WIDTH_; x++) {
			iterate(x, y, maxIterations_, greyonly);
		}
	}
#else
	const size_t numThreads = std::thread::hardware_concurrency();
	if(HEIGHT_ > (numThreads * 2)) {
		dim_t sliceHeight = std::floor(float(HEIGHT_) / numThreads);
		dim_t remainder = HEIGHT_ % sliceHeight;
		std::vector<std::thread*> threads;

		for(size_t i = 0; i < numThreads; ++i) {
			if(i == (numThreads - 1) && remainder > 0)
				sliceHeight += remainder;

			std::thread* t = new std::thread([=](){
				for (dim_t y = sliceHeight * i; y < (sliceHeight * (i + 1)); y++) {
					for (dim_t x = 0; x < WIDTH_; x++) {
						iterate(x, y, maxIterations_, greyonly);
					}
				}
			});
			threads.push_back(t);
		}
		for(auto& t : threads) {
			t->join();
			delete t;
		}
	} else {
		for (dim_t y = 0; y < HEIGHT_; y++) {
			for (dim_t x = 0; x < WIDTH_; x++) {
				iterate(x, y, maxIterations_, greyonly);
			}
		}
	}
#endif
}

// Calculate the color of a specific pixel
void Renderer::iterate(const coord_t& x, const coord_t& y, const uint64_t& maxiterations, const bool& greyonly) {
		using std::complex;
		float_t xViewport = (x + offsetx_ + panx_) / (zoom_ / 10);
		float_t yViewport = (y + offsety_ + pany_) / (zoom_ / 10);
    complex<float_t> point(xViewport/WIDTH_, yViewport/HEIGHT_);
    complex<float_t> z(0, 0);
    uint64_t iterations = 0;

    while (abs(z) < 2 && iterations < maxIterations_) {
        z = z * z + point;
        ++iterations;
    }

  	color24_t color;
  	size_t index = 0;
  	if (iterations == maxiterations) {
  		color = {0, 0, 0}; // Black
  	} else {
  		// 254 so we can use 0 as index for black
  		index = std::floor((float(iterations) / (maxiterations - 1)) * 254.0);
  		color = palette_[index];
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
void Renderer::zoomAt(const coord_t& x, const coord_t& y, const float_t& factor, const bool& zoomin) {
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

void Renderer::pan(const coord_t& x, const coord_t& y) {
	panx_ += x;
	pany_ += y;
}
} /* namespace fractaldive */
