#include <cstdint>
#include <cstddef>
#include <cmath>
#include <deque>
#include <vector>
#include <cstring>
#include <mutex>


#include "types.hpp"
#include "threadpool.hpp"

#ifndef SRC_RENDERER_HPP_
#define SRC_RENDERER_HPP_
#include "config.hpp"
#include "camera.hpp"

namespace fractaldive {

class Renderer {
public:
	Config& config_;
	Camera& camera_;
	const fd_dim_t BUFFERSIZE;
private:
	fd_iter_count_t maxIterations_;

public:
	image_t const imageData_;
	std::vector<uint32_t> palette_;

	Renderer(Config& config, Camera& camera, const fd_iter_count_t& maxIterations) :
			config_(config),
			camera_(camera),
			BUFFERSIZE(config.width_ * config.height_),
			maxIterations_(maxIterations),
			imageData_(new fd_image_pix_t[BUFFERSIZE]),
			palette_(makePalette()){
			memset(imageData_, 0, BUFFERSIZE * sizeof(fd_image_pix_t));
	}

	virtual ~Renderer() {
		delete[] imageData_;
	}
	inline fd_iter_count_t getCurrentMaxIterations() const;
	inline size_t calculatePaletteIndex(const fd_iter_count_t& iterations) const;
	inline const fd_image_pix_t& colorPixelAt(const size_t& index);
	inline fd_mandelfloat_t square(const fd_mandelfloat_t& n) const;
	inline fd_iter_count_t mandelbrot(const fd_coord_t& x, const fd_coord_t& y, const fd_iter_count_t& currentIt) const;


	void render();
	void zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin);
	void resetSmoothPan();
	void initSmoothPan(const fd_coord_t& x, const fd_coord_t& y);
	void pan(const fd_coord_t& x, const fd_coord_t& y);

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
