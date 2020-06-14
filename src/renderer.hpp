#include <cstdint>
#include <cstddef>
#include <cmath>
#include <deque>
#include <mutex>
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
	fd_float_t defaultZoom_;
	fd_float_t zoom_;
	// used for smoothing automatic panning
	size_t panSmoothLen_;
	std::deque<fd_coord_t> panHistoryX_;
	std::deque<fd_coord_t> panHistoryY_;
public:
	image_t const imgdata_;
	shadow_image_t const shadowData_;

	Renderer(const fd_dim_t& width, const fd_dim_t& height, const fd_iter_count_t& maxIterations, const fd_float_t& zoomFactor, const size_t& panSmoothLen) :
			WIDTH_(width),
			HEIGHT_(height),
			BUFFERSIZE(width * height),
			maxIterations_(maxIterations),
			offsetx_(-fd_float_t(width)/2.0),
			offsety_(-fd_float_t(height)/2.0),
			defaultZoom_(zoomFactor),
			zoom_(zoomFactor),
			panSmoothLen_(panSmoothLen),
			imgdata_(new fd_image_pix_t[width * height]),
#ifndef _NO_SHADOW
			shadowData_(new fd_shadow_pix_t[width * height]) {
#else
			shadowData_(imgdata_) {
#endif
	}

	virtual ~Renderer() {
		delete[] imgdata_;
#ifndef _NO_SHADOW
		delete[] shadowData_;
#endif
	}

	size_t calculatePaletteIndex(const fd_iter_count_t& iterations,	const fd_iter_count_t& maxIterations) const;
	void colorPixelAt(const fd_coord_t& x, const fd_coord_t& y, const size_t& index, const fd_iter_count_t& iterations, const bool& shadowOnly);
	inline fd_mandelfloat_t square(const fd_mandelfloat_t& n) const;
	inline fd_iter_count_t mandelbrot(const fd_coord_t& x, const fd_coord_t& y) const;
	fd_iter_count_t iterate(const fd_coord_t& x, const fd_coord_t& y) const;

	void reset() {
	  offsetx_ = -fd_float_t(WIDTH_)/2.0;
		offsety_ = -fd_float_t(WIDTH_)/2.0;
		panx_ = 0;
		pany_ = 0;
		zoom_ = defaultZoom_;
		panHistoryX_.clear();
		panHistoryY_.clear();
	}

	void render(const bool& shadowonly = false);
	void zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin);
	void resetSmoothPan();
	void initSmoothPan(const fd_coord_t& x, const fd_coord_t& y);
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

private:
	std::pair<fd_coord_t, fd_coord_t> smoothPan(const fd_coord_t& x, const fd_coord_t& y);

};
} /* namespace fractaldive */

#endif /* SRC_RENDERER_HPP_ */
