#include <cstdint>
#include <cstddef>
#include <cmath>

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
	fd_iter_count_t maxIterations_;
	fd_coord_t offsetx_;
	fd_coord_t offsety_;

	// Pan and zoom parameters
	fd_coord_t panx_ = 0;
	fd_coord_t pany_ = 0;
	fd_float_t zoom_ = 2;
public:
	image_t imgdata_;
	shadow_image_t shadowdata_;

	Renderer(const fd_dim_t& width, const fd_dim_t& height, const fd_iter_count_t& maxIterations) :
			WIDTH_(width),
			HEIGHT_(height),
			BUFFERSIZE(width * height),
			maxIterations_(maxIterations),
			offsetx_(-fd_float_t(width)/2.0),
			offsety_(-fd_float_t(height)/2.0),
			imgdata_(new fd_image_comp_t[width * height]),
#ifndef _NO_SHADOW
			shadowdata_(new fd_shadow_comp_t[width * height]) {
#else
			shadowdata_(imgdata_) {
#endif
	}

	virtual ~Renderer() {
		delete[] imgdata_;
#ifndef _NO_SHADOW
		delete[] shadowdata_;
#endif
	}

	void colorPixelByPalette(const fd_coord_t& x, const fd_coord_t& y, const fd_iter_count_t iterations, const bool& shadowonly);
	inline fd_mandelfloat_t square(const fd_mandelfloat_t& n) const;
	inline fd_iter_count_t mandelbrot(const fd_coord_t& x, const fd_coord_t& y) const;
	void iterate(const fd_coord_t& x, const fd_coord_t& y, const bool& shadowonly);

	void reset() {
	  offsetx_ = -fd_float_t(WIDTH_)/2.0;
		offsety_ = -fd_float_t(WIDTH_)/2.0;
		panx_ = 0;
		pany_ = 0;
		zoom_ = 2;
	}

	void render(const bool& shadowonly = false);
	void zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin);
	void pan(const fd_coord_t& x, const fd_coord_t& y);

	fd_float_t getZoom() const {
		return zoom_;
	}

	fd_iter_count_t getMaxIterations() const {
		return maxIterations_;
	}

	void setMaxIterations(const fd_iter_count_t& mi) {
		maxIterations_ = mi;
	}
};
} /* namespace fractaldive */

#endif /* SRC_RENDERER_HPP_ */
