/*
 * camera.cpp
 *
 *  Created on: Jan 16, 2021
 *      Author: elchaschab
 */

#include "camera.hpp"

namespace fractaldive {

Camera::~Camera() {
}

std::pair<fd_coord_t, fd_coord_t> Camera::calculatePanVector(const fd_coord_t& x, const fd_coord_t& y) {
	fd_float_t hDiff = x - std::floor(config_.width_ / 2.0);
	fd_float_t vDiff = y - std::floor(config_.height_ / 2.0);
	fd_float_t scale = (1.0 - (1.0 / (config_.width_ / 8.0)))
			* ((config_.fps_ + (config_.fps_ * config_.zoomSpeed_)) / (config_.fps_ / (config_.zoomSpeed_ * 0.1)));
	fd_coord_t panX = (hDiff * scale);
	fd_coord_t panY = (vDiff * scale);
	return {panX, panY};
}

void Camera::zoom(fd_coord_t atX, fd_coord_t atY) {
	const auto& pv = calculatePanVector(atX, atY);
	fd_float_t zoomFactor = 1.0 + (config_.zoomSpeed_ / config_.fps_);
	pan(pv.first, pv.second);
	zoomAt(config_.width_ / 2.0, config_.height_ / 2.0, zoomFactor, true);
	++frameCount_;
}

// Zoom the fractal
void Camera::zoomAt(const fd_coord_t& x, const fd_coord_t& y, const fd_float_t& factor, const bool& zoomin) {
	if (zoomin) {
		// Zoom in
		++zoomCount_;
		zoom_ *= factor;
		panx_ = factor * (x + offsetx_ + panx_);
		pany_ = factor * (y + offsety_ + pany_);
	} else {
		// Zoom out
		--zoomCount_;
		zoom_ /= factor;
		panx_ = (x + offsetx_ + panx_) / factor;
		pany_ = (y + offsety_ + pany_) / factor;
	}
}

void Camera::resetSmoothPan() {
	panHistoryX_.clear();
	panHistoryY_.clear();
}

void Camera::initSmoothPan(const fd_coord_t& x, const fd_coord_t& y) {
	panHistoryX_.resize(panSmoothLen_);
	panHistoryY_.resize(panSmoothLen_);

	fd_float_t xstep = (x / panSmoothLen_);
	fd_float_t ystep = (y / panSmoothLen_);

	for (size_t i = 0; i < panSmoothLen_; ++i) {
		panHistoryX_[i] = (xstep * i);
	}

	for (size_t i = 0; i < panSmoothLen_; ++i) {
		panHistoryY_[i] = (ystep * i);
	}
}

std::pair<fd_coord_t, fd_coord_t> Camera::smoothPan(const fd_coord_t& x, const fd_coord_t& y) {
	assert((panHistoryX_.empty() && panHistoryY_.empty()) || (!panHistoryX_.empty() && !panHistoryY_.empty()));
	if (panHistoryX_.empty()) {
		initSmoothPan(x, y);
	}

	panHistoryX_.pop_back();
	panHistoryY_.pop_back();
	panHistoryX_.push_front(x);
	panHistoryY_.push_front(y);

	fd_coord_t xhtotal = 0;

	for (const auto& xh : panHistoryX_) {
		xhtotal += xh;
	}

	fd_coord_t yhtotal = 0;
	for (const auto& yh : panHistoryY_) {
		yhtotal += yh;
	}

	return {fd_float_t(xhtotal) / panSmoothLen_, fd_float_t(yhtotal) / panSmoothLen_};
}

// Pan the fractal
void Camera::pan(const fd_coord_t& x, const fd_coord_t& y) {
	auto ft = smoothPan(x, y);
	panx_ += ft.first;
	pany_ += ft.second;
}
} /* namespace fractaldive */
