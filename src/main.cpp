/*#include <cwchar>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <unistd.h>
#include <sys/time.h>
*/

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
#include "imagedetail.hpp"

using namespace fractaldive;

bool do_run = true;
Config& config = Config::getInstance();
Renderer renderer(config.width_, config.height_, config.startIterations_, config.zoomFactor_, config.panSmoothLen_);
Canvas canvas(config.width_, config.height_, false);

std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfHighestDetail(const fd_coord_t& tiling) {
	assert(tiling > 1);
	const fd_coord_t tileW = std::floor(fd_float_t(config.width_) / fd_float_t(tiling));
	const fd_coord_t tileH = std::floor(fd_float_t(config.height_) / fd_float_t(tiling));
	assert(tileW > 1);
	assert(tileH > 1);

	const size_t tileSize = tileW * tileH;

	const auto& image = renderer.imageData_;
	std::vector<fd_image_pix_t> tile(tileSize);
	std::vector<fd_float_t> tileScores;
	fd_float_t candidateScore = 0;
	fd_coord_t candidateTx = 0;
	fd_coord_t candidateTy = 0;

	//iterate over tiles
	for (fd_coord_t ty = 0; ty < tiling; ++ty) {
		for (fd_coord_t tx = 0; tx < tiling; ++tx) {
			const fd_coord_t offY = tileH * ty * config.width_;
			const fd_coord_t offX = tileW * tx;

			//iterate over pixels of the tile
			for (fd_coord_t y = 0; y < tileH; ++y) {
				for (fd_coord_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * config.width_)) + (offX + x);
					tile[y * tileW + x] = image[pixIdx];
				}
			}
			fd_float_t detail = measureImageDetail(tile.data(), tileW * tileH);

			if (detail > candidateScore) {
				candidateScore = detail;
				candidateTx = tx;
				candidateTy = ty;
			}
		}
	}

	return { (candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2) };
}

std::pair<fd_coord_t, fd_coord_t> calculatePanVector(const fd_coord_t& x, const fd_coord_t& y) {
	fd_float_t hDiff = x - std::floor(config.width_ / 2.0);
	fd_float_t vDiff = y - std::floor(config.height_ / 2.0);
	fd_float_t scale = (1.0 - (1.0 / (config.width_ / 8.0)))
			* ((config.fps_ + (config.fps_ * config.zoomSpeed_)) / (config.fps_ / (config.zoomSpeed_ * 0.1)));
	fd_coord_t panX = (hDiff * scale);
	fd_coord_t panY = (vDiff * scale);
	return {panX, panY};
}

bool dive(bool zoom, bool benchmark) {
	fd_float_t detail = measureImageDetail(renderer.imageData_, config.frameSize_);

	if (!benchmark && detail < config.detailThreshold_) {
#ifdef _JAVASCRIPT
		renderer.reset();
		renderer.resetSmoothPan();
		renderer.render();
		//		emscripten_cancel_main_loop();
#endif
		return false;
	}
	if (zoom) {
		std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail;
		if(detail < config.findDetailThreshold_) {
			centerOfHighDetail = identifyCenterOfTileOfHighestDetail(config.frameTiling_);
		} else {
			centerOfHighDetail = {config.width_ / 2.0, config.height_ / 2.0};
		}
		const auto& pv = calculatePanVector(centerOfHighDetail.first, centerOfHighDetail.second);
		fd_float_t zoomFactor = 1.0 + (config.zoomSpeed_ / config.fps_);
		renderer.pan(pv.first, pv.second);
		renderer.zoomAt(config.width_ / 2.0, config.height_ / 2.0, zoomFactor, true);
	}

	renderer.render();
//	if(!benchmark)
		canvas.draw(renderer.imageData_);
	return true;
}

bool auto_scale_max_iterations() {
	auto start = get_milliseconds();
	auto duration = start;

	size_t cnt = 0;
	while ((duration = (get_milliseconds() - start)) < config.benchmarkTimeoutMillis_) {
		dive(false, true);
		++cnt;
	}

	renderer.reset();
	fd_float_t fpsMillis = 1000.0 / config.fps_;
	fd_float_t exp = 1.2 + config.frameSize_ * (0.12 / (128 * 128));
	fd_float_t millisRatio = (pow(duration / (cnt / 20.0), exp) / fpsMillis);
#ifndef _FIXEDPOINT
	fd_iter_count_t iterations = std::round((config.startIterations_ / millisRatio) * 10.0);
#else
	fd_iter_count_t iterations = std::round((config.startIterations_.ToFloat() / millisRatio) * 10.0);
#endif

#ifdef _JAVASCRIPT_MT
	iterations = (iterations * (ThreadPool::cores()));
#endif

#ifdef _JAVASCRIPT
	iterations *= 0.7;
#endif

	print(iterations);
#ifdef _BENCHMARK_ONLY
	return true;
#endif
	if (iterations < config.minIterations_)
		config.fps_ = std::max(std::floor(config.fps_ * (fd_float_t(iterations) / config.minIterations_)), 1.0);
	iterations = std::min(config.maxIterations_, std::max(iterations, config.minIterations_));
	renderer.setMaxIterations(iterations);
	return false;
}

bool step() {
	auto start = get_milliseconds();
	bool result = dive(true, false);
	auto duration = get_milliseconds() - start;

	int32_t targetMillis = 1000.0 / config.fps_;
	int32_t diff = targetMillis - duration;

	if (result) {
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
}
#endif

void printReport() {
	print("#####");
	print("# SETTINGS");
	size_t padWidth = 20;
	print(pad_string("Width:", padWidth), config.width_);
	print(pad_string("Height:", padWidth), config.height_);
	print(pad_string("Zoom speed:", padWidth), config.zoomSpeed_);
	print(pad_string("Benchmark timeout:", padWidth), config.benchmarkTimeoutMillis_, "ms");

	print("");
	print("# FEATURES");
	print(pad_string("Threads:", padWidth), ThreadPool::cores());
#ifdef _AUTOVECTOR
	print(pad_string("Auto Vector/SIMD:", padWidth),"on");
#else
	print(pad_string("Auto Vector/SIMD:", padWidth), "off");
#endif

#ifdef _FIXEDPOINT
	print(pad_string("Arithmetic:", padWidth),"fixed point");
#else
	print(pad_string("Arithmetic:", padWidth), "floating point");
#endif
	print(pad_string("Precision:", padWidth), FD_PRECISION);
	print("");

	print("# SCALING");
	print(pad_string("FPS:", padWidth), config.fps_);
	print(pad_string("Max iterations:", padWidth), renderer.getMaxIterations(), "of", config.maxIterations_);
	print(pad_string("Detail threshold:", padWidth), config.detailThreshold_);
	print(pad_string("Pan history:", padWidth), config.panSmoothLen_);
	print("#####");
	print("");
}

void run() {
	if(auto_scale_max_iterations()){
			do_run = false;
#ifndef _JAVASCRIPT
			ThreadPool::getInstance().stop();
#endif
	}	else {
		printReport();
	}

	fd_highres_tick_t start = 0;
	while (do_run) {
		start = get_milliseconds();
		renderer.reset();
		renderer.resetSmoothPan();
		renderer.pan(0, 0);
		renderer.render();
		fd_float_t detail = measureImageDetail(renderer.imageData_, config.frameSize_);

		if (detail > config.detailThreshold_) {
#ifdef _JAVASCRIPT
			emscripten_set_main_loop(js_step, 0, 1);
#else
			bool stepResult = true;
			while (do_run && stepResult) {
				stepResult = step();
			}
#endif
			print("Duration:", (get_milliseconds() - start) / 1000.0, "seconds");
		} else {
			print("Skip:", detail);
			sleep_millis(1000.0 / config.fps_);
		}
	}
//	std::cout << renderer.getMaxIterations() << "/" << renderer.getZoomCount() << std::endl;

//#ifndef _NO_THREADS
//	ThreadPool::getInstance().stop();
//#endif
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
	assert(config.width_ >= 16 && config.width_ % 2 == 0);
	assert(config.height_ >= 16 && config.height_ % 2 == 0);
	assert(config.startIterations_ > 3);
	assert(config.fps_ > 0);

	srand(time(NULL));
#ifndef _JAVASCRIPT
#ifndef _AMIGA
	signal(SIGINT, sigint_handler);
#endif
#endif
	run();
	return 0;
}
