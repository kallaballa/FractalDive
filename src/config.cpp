#include "config.hpp"

namespace fractaldive {

Config* Config::instance_ = nullptr;

Config::Config() {
	resetToDefaults();
}

Config::~Config() {
}

void Config::resetToDefaults() {
	fps_ = 24;
	minIterations_ = 10;
	benchmarkTimeoutMillis_ = 3000;
	detailThreshold_ = 0.02;
	startIterations_ = 100;
	frameTiling_ = 5;
	zoomFactor_ = 2;
	panSmoothLen_ = 20;
	findDetailThreshold_ = 0.1;
#ifndef _AMIGA
#ifdef _LOW_RES
	width_ = 128;
	height_ = 128;
#else
#ifdef _HIGH_RES
	width_ = 384;
	height_ = 384;
#else
#ifdef _ULTRA_RES
	width_ = 768;
	height_ = 768;
#else
	width_ = 256;
	height_ = 256;
#endif
#endif
#endif
	maxIterations_ = (width_ * height_) / 24;
#else
	width_ = 52;
	height_ = 52;
	panSmoothLen_ = 10;
	maxIterations_ = 300;
	benchmarkTimeoutMillis_ = 1000;
#endif
	frameSize_ = width_ * height_;

#ifdef _FAST_ZOOM
	zoomSpeed_ = 1;
#else
	#ifdef _FASTER_ZOOM
	zoomSpeed_ = 3;
	#else
		#ifdef _SLOW_ZOOM
	zoomSpeed_ = 0.2;
		#else
	zoomSpeed_ = 0.5;
		#endif
	#endif
#endif


}
} /* namespace fractaldive */
