#include <cwchar>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <unistd.h>
#include <sys/time.h>

#include <map>
#include <vector>
#include <set>

#ifndef _JAVASCRIPT
#include <csignal>
#else
#include <emscripten.h>
#endif

#ifdef _AMIGA
#include "amiga.hpp"
#endif

#include "printer.hpp"
#include "threadpool.hpp"
#include "config.hpp"
#include "renderer.hpp"
#include "canvas.hpp"
#include "util.hpp"
#include "tilingkernel.hpp"
#include "imagedetail.hpp"

using namespace fractaldive;

Config& CONFIG = Config::getInstance();
Renderer RENDERER(CONFIG.width_, CONFIG.height_, CONFIG.startIterations_, CONFIG.zoomFactor_, CONFIG.panSmoothLen_);
Canvas CANVAS(CONFIG.width_, CONFIG.height_, false);
TilingKernel<5> tkernel;

bool do_run = true;



std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfHighestDetail(const fd_coord_t& tiling) {
	assert(tiling > 1);
	const fd_coord_t tileW = std::floor(fd_float_t(CONFIG.width_) / fd_float_t(tiling));
	const fd_coord_t tileH = std::floor(fd_float_t(CONFIG.height_) / fd_float_t(tiling));
	const size_t tileSize = tileW * tileH;
	assert(tileW > 1);
	assert(tileH > 1);

	const auto& image = RENDERER.imageData_;
	std::vector<fd_image_pix_t> tile(tileSize);

	std::vector<fd_float_t> tileScores;
	fd_float_t candidateScore = 0;
	fd_coord_t candidateTx = 0;
	fd_coord_t candidateTy = 0;

	//iterate over tiles
	for (fd_coord_t ty = 0; ty < tiling; ++ty) {
		for (fd_coord_t tx = 0; tx < tiling; ++tx) {
			const fd_coord_t offY = tileH * ty * CONFIG.width_;
			const fd_coord_t offX = tileW * tx;

			//iterate over pixels of the tile
			for (fd_coord_t y = 0; y < tileH; ++y) {
				for (fd_coord_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * CONFIG.width_)) + (offX + x);
					tile[y * tileW + x] = image[pixIdx];
				}
			}
			fd_float_t detail = measureImageDetail(tile.data(), tileW * tileH);
  		fd_float_t weight = tkernel[tx][ty];
			fd_float_t score =  detail * weight;

			if (score > candidateScore) {
				candidateScore = score;
				candidateTx = tx;
				candidateTy = ty;
			}
		}

	}

	std::pair<fd_coord_t, fd_coord_t> center = {(candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2)};
	return center;
}

std::pair<fd_coord_t,fd_coord_t> calculatePanVector(const fd_coord_t& x, const fd_coord_t& y) {
	fd_float_t hDiff = x - std::floor(CONFIG.width_ / 2.0);
	fd_float_t vDiff = y - std::floor(CONFIG.height_ / 2.0);
	fd_float_t scale = (1.0 - (1.0 / (CONFIG.width_ / 10.0))) * ((CONFIG.fps_ + (CONFIG.fps_ * CONFIG.zoomSpeed_)) / (CONFIG.fps_ / (CONFIG.zoomSpeed_ * 0.1)));
	fd_coord_t panX = (hDiff * scale);
	fd_coord_t panY = (vDiff * scale);
	return {panX, panY};
}

bool dive(bool zoom, bool benchmark) {
		fd_float_t detail = measureImageDetail(RENDERER.imageData_, CONFIG.frameSize_);

		if (!benchmark && detail < CONFIG.detailThreshold_) {
#ifdef _JAVASCRIPT
			RENDERER.reset();
			RENDERER.resetSmoothPan();
			RENDERER.render();
			//		emscripten_cancel_main_loop();
#endif
			return false;
		}
		if (zoom) {
			std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail;
				centerOfHighDetail = identifyCenterOfTileOfHighestDetail(CONFIG.frameTiling_);
				const auto& pv = calculatePanVector(centerOfHighDetail.first, centerOfHighDetail.second);
				fd_float_t zoomFactor = 1.0 + (CONFIG.zoomSpeed_ / CONFIG.fps_);
				RENDERER.pan(pv.first, pv.second);
				RENDERER.zoomAt(CONFIG.width_ / 2.0, CONFIG.height_ / 2.0, zoomFactor, true);
		}

		RENDERER.render();

			CANVAS.draw(RENDERER.imageData_);
	return true;
}

void auto_scale_max_iterations() {

	auto start = get_milliseconds();
	auto duration = start;

	size_t cnt = 0;
	while ((duration = (get_milliseconds() - start)) < CONFIG.benchmarkTimeoutMillis_) {
		dive(false, true);
		++cnt;
	}

 	RENDERER.reset();
	fd_float_t fpsMillis = 1000.0 / CONFIG.fps_;
	fd_float_t exp = 1.2 + CONFIG.frameSize_ * (0.12 / (128 * 128));
	fd_float_t millisRatio = (pow(duration / (cnt / 20.0), exp) / fpsMillis);
#ifndef _FIXEDPOINT
	fd_iter_count_t iterations = std::round((CONFIG.startIterations_ / millisRatio) * 10.0);
#else
	fd_iter_count_t iterations = std::round((CONFIG.startIterations_.ToFloat() / millisRatio) * 10.0);
#endif

#ifdef _JAVASCRIPT_MT
	iterations = (iterations * (ThreadPool::cores()));
#endif

#ifdef _JAVASCRIPT
	iterations *= 0.7;
#endif

	print("# Score");
	print(iterations);

	if(iterations < CONFIG.minIterations_)
			CONFIG.fps_ = std::max(std::floor(CONFIG.fps_ * (fd_float_t(iterations) / CONFIG.minIterations_)),1.0);
	iterations = std::min(CONFIG.maxIterations_, std::max(iterations, CONFIG.minIterations_));
	RENDERER.setMaxIterations(iterations);
}

bool step() {
	auto start = get_milliseconds();
	bool result = dive(true, false);
	auto duration = get_milliseconds() - start;

	int32_t targetMillis = 1000.0 / CONFIG.fps_;
	int32_t diff = targetMillis - duration;

	if(result) {
		if (diff > 0) {
			sleep_millis(diff);
		} else if (diff < 0)
			printErr("Underrun: ", std::abs(diff));
	}
	return result;
}

#ifdef _JAVASCRIPT
void js_step() {
	step();
#ifndef _NO_TIMETRACK
	printErr(TimeTracker::getInstance().str());
	TimeTracker::getInstance().epoch();
#endif
}
#endif

void printReport() {
	print("#####");
	print("# SETTINGS");
	size_t padWidth = 20;
	print(pad_string("Width:", padWidth), CONFIG.width_);
	print(pad_string("Height:", padWidth), CONFIG.height_);
	print(pad_string("Zoom speed:", padWidth), CONFIG.zoomSpeed_);
	print(pad_string("Benchmark timeout:", padWidth), CONFIG.benchmarkTimeoutMillis_, "ms");

	print("");
	print("# FEATURES");
	print(pad_string("Threads:", padWidth), ThreadPool::cores());
#ifdef _AUTOVECTOR
	print(pad_string("Auto Vector/SIMD:", padWidth),"on");
#else
	print(pad_string("Auto Vector/SIMD:", padWidth),"off");
#endif

#ifdef _FIXEDPOINT
	print(pad_string("Arithmetic:", padWidth),"fixed point");
#else
	print(pad_string("Arithmetic:", padWidth),"floating point");
#endif
	print(pad_string("Precision:", padWidth),FD_PRECISION);
	print("");

	print("# SCALING");
	print(pad_string("FPS:", padWidth), CONFIG.fps_);
	print(pad_string("Max iterations:", padWidth), RENDERER.getMaxIterations(), "of", CONFIG.maxIterations_);
	print(pad_string("Detail threshold:", padWidth), CONFIG.detailThreshold_);
	print(pad_string("Pan history:", padWidth), CONFIG.panSmoothLen_);
	print("#####");

	print("");

}

void run() {
	auto_scale_max_iterations();
	printReport();

	fd_highres_tick_t start = 0;
	while (do_run) {
		start = get_milliseconds();
		tkernel.initAt(rand() % CONFIG.frameTiling_, rand() % CONFIG.frameTiling_);
		RENDERER.reset();
		RENDERER.resetSmoothPan();
		RENDERER.pan(0,0);
		RENDERER.render();
		fd_float_t detail = measureImageDetail(RENDERER.imageData_, CONFIG.frameSize_);
		if(detail > CONFIG.detailThreshold_) {
#ifdef _JAVASCRIPT
			emscripten_set_main_loop(js_step, 0, 1);
#else

			bool stepResult = true;
			while (do_run && stepResult) {
				stepResult = step();
#ifndef _NO_TIMETRACK
				printErr(tt.str());
				tt.epoch();
#endif
			}
#endif
			print("Duration:", (get_milliseconds() - start) / 1000.0, "seconds");
		} else {
			print("Skip:", detail);
			sleep_millis(1000.0 / CONFIG.fps_);
		}
	}
#ifndef _NO_THREADS
	ThreadPool::getInstance().stop();
#endif
	SDL_Quit();
}

//does report true for 0
bool is_power_of_two(const fd_coord_t& x) {
	return (x & (x - 1)) == 0;
}

#ifndef _JAVASCRIPT
#ifndef _AMIGA
void sigint_handler(int sig) {
	do_run = false;
}
#endif
#endif

int main() {
	assert(CONFIG.width_ >= 16 && is_power_of_two(CONFIG.width_));
	assert(CONFIG.height_ >= 16 && is_power_of_two(CONFIG.height_));
	assert(CONFIG.startIterations_ > 3);
	assert(CONFIG.fps_ > 0);

	srand(time(NULL));
#ifndef _JAVASCRIPT
#ifndef _AMIGA
	signal(SIGINT, sigint_handler);
#endif
#endif
	run();
	return 0;
}
