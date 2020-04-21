#include <cstdint>
#include <cstddef>
#include <vector>

#include "color.hpp"

#ifndef SRC_RENDERER_HPP_
#define SRC_RENDERER_HPP_

namespace fractaldive {

class Renderer {
public:
	const size_t WIDTH_;
	const size_t HEIGHT_;
	const size_t BUFFERSIZE;
private:
	Color palette_[256];
	size_t maxIterations_;
	int32_t offsetx_;
	int32_t offsety_;

	// Pan and zoom parameters
	int32_t panx_ = 0;
	int32_t pany_ = 0;
	double zoom_ = 2;
	void generatePalette();
	void iterate(int32_t x, int32_t y, size_t maxiterations, bool greyonly);
public:
	Color* rgbdata_;
	uint8_t* greydata_;

	Renderer(const size_t& width, const size_t& height, const size_t& maxIterations) :
			WIDTH_(width),
			HEIGHT_(height),
			BUFFERSIZE(width * height),
			maxIterations_(maxIterations),
			offsetx_(-float(width)/2.0),
			offsety_(-float(height)/2.0),
			rgbdata_(new Color[width * height]),
			greydata_(new uint8_t[width * height])
{
		generatePalette();
	}

	virtual ~Renderer() {
	}

	void render(bool greyonly = false);
	void zoomAt(int32_t x, int32_t y, double factor, bool zoomin);
	void zoomAtCenter(double factor, bool zoomin);
	void pan(const int32_t& x, const int32_t& y);
	size_t getMaxIterations() {
		return maxIterations_;
	}

	void setMaxIterations(size_t mi) {
		maxIterations_ = mi;
	}
};
} /* namespace fractaldive */

#endif /* SRC_RENDERER_HPP_ */
