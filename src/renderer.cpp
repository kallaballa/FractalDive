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
	size_t increment = Palette::SIZE_ / 256;
	for (size_t i = 0; i < 256; i++) {
		palette_[i] = Palette::ITEMS_[i * increment];
	}
}

// Generate the fractal image
void Renderer::render(bool greyonly) {
	// Iterate over the pixels
#ifndef _JAVASCRIPT
	for (size_t y = 0; y < HEIGHT_; y++) {
		#pragma omp parallel for ordered schedule(dynamic)
		for (size_t x = 0; x < WIDTH_; x++) {
			iterate(x, y, maxIterations_, greyonly);
		}
	}
#else
	const size_t numThreads = 8;
	if(HEIGHT_ > (numThreads * 2)) {
		size_t sliceHeight = std::floor(float(HEIGHT_) / numThreads);
		size_t remainder = HEIGHT_ % sliceHeight;
		std::vector<std::thread*> threads;

		for(size_t i = 0; i < numThreads; ++i) {
			if(i == (numThreads - 1) && remainder > 0)
				sliceHeight += remainder;

			std::thread* t = new std::thread([=](){
				for (size_t y = sliceHeight * i; y < (sliceHeight * (i + 1)); y++) {
					for (size_t x = 0; x < WIDTH_; x++) {
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
		for (size_t y = 0; y < HEIGHT_; y++) {
			for (size_t x = 0; x < WIDTH_; x++) {
				iterate(x, y, maxIterations_, greyonly);
			}
		}
	}
#endif
}

// Calculate the color of a specific pixel
void Renderer::iterate(int32_t x, int32_t y, size_t maxiterations, bool greyonly) {
		using std::complex;
		float xViewport = (x + offsetx_ + panx_) / (zoom_ / 10);
		float yViewport = (y + offsety_ + pany_) / (zoom_ / 10);
    complex<float> point(xViewport/WIDTH_, yViewport/HEIGHT_);
    complex<float> z(0, 0);
    size_t iterations = 0;
    while (abs(z) < 2 && iterations < maxIterations_) {
        z = z * z + point;
        ++iterations;
    }

  	Color color;
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
void Renderer::zoomAt(int32_t x, int32_t y, double factor, bool zoomin) {
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

void Renderer::zoomAtCenter(double factor, bool zoomin) {
	if (zoomin) {
		// Zoom in
		zoom_ *= factor;
	} else {
		// Zoom out
		zoom_ /= factor;
	}
}

void Renderer::pan(const int32_t& x, const int32_t& y) {
	panx_ += x;
	pany_ += y;
}
} /* namespace fractaldive */
