/*
 * config.cpp
 *
 *  Created on: Jun 14, 2020
 *      Author: elchaschab
 */

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
	benchmarkTimeoutMillis_ = 1000;
	detailThreshold_ = 0.001;
	startIterations_ = 100;
	panSmoothLen_ = 20;
	#ifndef _AMIGA
#ifdef _LOW_RES
	width_ = 128;
	height_ = 128;
	maxIterations_ = 500;
#else
#ifdef _HIGH_RES
	width_ = 512;
	height_ = 512;
	maxIterations_ = 10000;
#else
	width_ = 256;
	height_ = 256;
	maxIterations_ = 3000;
#endif
#endif
#else
	width_ = 30;
	height_ = 30;
	panSmoothLen_ = 2;
	maxIterations_ = 100;
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
	zoomSpeed_ = 0.45;
		#endif
	#endif
#endif


}
} /* namespace fractaldive */
