#include <cstdint>
#include <cstddef>
#include <vector>

#include "types.hpp"

#ifndef SRC_RENDERER_HPP_
#define SRC_RENDERER_HPP_

namespace fractaldive {

class Renderer {
public:
	const fd_dim_t WIDTH_;
	const fd_dim_t HEIGHT_;
	const fd_dim_t BUFFERSIZE;
private:
	color24_t palette_[256];
	uint64_t initialMaxIterations_;
	uint64_t maxIterations_;
	fd_coord_t offsetx_;
	fd_coord_t offsety_;

	// Pan and zoom parameters
	fd_coord_t panx_ = 0;
	fd_coord_t pany_ = 0;
	fd_float_t zoom_ = 2;

	void generatePalette();
	void iterate(const fd_coord_t& x, const fd_coord_t& y, const uint64_t& maxiterations, const bool& greyonly);
public:
	rgb_image_t rgbdata_;
	grey_image_t greydata_;

	Renderer(const fd_dim_t& width, const fd_dim_t& height, const uint64_t& maxIterations) :
			WIDTH_(width),
			HEIGHT_(height),
			BUFFERSIZE(width * height),
			initialMaxIterations_(maxIterations),
			maxIterations_(maxIterations),
			offsetx_(-fd_float_t(width)/2.0),
			offsety_(-fd_float_t(height)/2.0),
			rgbdata_(new color24_t[width * height]),
			greydata_(new fd_ccomp_t[width * height])
{
		generatePalette();
	}

	virtual ~Renderer() {
		delete[] rgbdata_;
		delete[] greydata_;
	}

	void reset() {
		maxIterations_ = initialMaxIterations_;
		offsetx_ = -fd_float_t(WIDTH_)/2.0;
		offsety_ = -fd_float_t(WIDTH_)/2.0;
		panx_ = 0;
		pany_ = 0;
		zoom_ = 2;
	}
	void render(bool greyonly = false);
	void zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin);
	void pan(const fd_coord_t& x, const fd_coord_t& y);

	fd_float_t getZoom() const {
		return zoom_;
	}

	uint64_t getMaxIterations() const {
		return maxIterations_;
	}

	void setMaxIterations(const uint64_t& mi) {
		maxIterations_ = mi;
	}
};
} /* namespace fractaldive */

#endif /* SRC_RENDERER_HPP_ */
