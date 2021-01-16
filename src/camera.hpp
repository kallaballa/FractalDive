#ifndef SRC_CAMERA_HPP_
#define SRC_CAMERA_HPP_

#include <utility>
#include <deque>
#include <cassert>
#include <cmath>

#include "config.hpp"

namespace fractaldive {

class Camera {
	Config& config_;
	fd_coord_t offsetx_;
	fd_coord_t offsety_;

	// Pan and zoom parameters
	fd_coord_t panx_ = 0;
	fd_coord_t pany_ = 0;
	fd_float_t defaultZoom_;
	fd_float_t zoom_;
	fd_float_t zoomCount_ = 0;
	fd_dim_t frameCount_ = 0;


	// used for smoothing automatic panning
	size_t panSmoothLen_;
	std::deque<fd_coord_t> panHistoryX_;
	std::deque<fd_coord_t> panHistoryY_;
public:
	Camera(Config& config, const fd_float_t& zoomFactor, const size_t& panSmoothLen) :
			config_(config),
			offsetx_(-fd_float_t(config.width_) / 2.0),
			offsety_(-fd_float_t(config.height_) / 2.0),
			defaultZoom_(zoomFactor),
			zoom_(zoomFactor),
			panSmoothLen_(panSmoothLen)
	{
	}
	virtual ~Camera();
	std::pair<fd_coord_t, fd_coord_t> calculatePanVector(const fd_coord_t& x, const fd_coord_t& y);
	void zoom(fd_coord_t atX, fd_coord_t atY);
	void zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin);
	void resetSmoothPan();
	void initSmoothPan(const fd_coord_t& x, const fd_coord_t& y);
	std::pair<fd_coord_t, fd_coord_t> smoothPan(const fd_coord_t& x, const fd_coord_t& y);
	void pan(const fd_coord_t& x, const fd_coord_t& y);
	void reset() {
		srand(time(NULL));
		offsetx_ = -fd_float_t(config_.width_) / 2.0;
		offsety_ = -fd_float_t(config_.height_) / 2.0;
		panx_ = 0;
		pany_ = 0;
		zoom_ = defaultZoom_;
		zoomCount_ = 0;
		frameCount_ = 0;
		panHistoryX_.clear();
		panHistoryY_.clear();
	}

	fd_float_t getZoomCount() const {
		return zoomCount_;
	}

	fd_float_t getFrameCount() const {
		return frameCount_;
	}

	fd_float_t getZoom() const {
		return zoom_;
	}

	fd_float_t getPanY() const {
		return pany_;
	}

	fd_float_t getPanX() const {
		return panx_;
	}

	fd_float_t getOffsetY() const {
		return offsety_;
	}

	fd_float_t getOffsetX() const {
		return offsetx_;
	}
};
} /* namespace fractaldive */

#endif /* SRC_CAMERA_HPP_ */
