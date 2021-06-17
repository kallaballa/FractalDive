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
#include "tilingkernel.hpp"
#include "camera.hpp"

using namespace fractaldive;

bool do_run = true;
Config& config = Config::getInstance();
Camera camera(config, config.zoomFactor_);
Renderer renderer(config, camera,config.startIterations_);
Canvas canvas(config.width_, config.height_, false);
TilingKernel<5> tkernel;

struct ZoomEvent {
	std::pair<size_t, size_t> zoomPoint_ = { 0, 0};
	bool active_ = false;
};

ZoomEvent current_zoom_event;
void process_events() {
	SDL_Event test_event;
	while (SDL_PollEvent(&test_event)) {
		switch (test_event.type) {
		case SDL_MOUSEBUTTONDOWN:
			current_zoom_event.zoomPoint_ = {test_event.motion.x, test_event.motion.y};
			current_zoom_event.active_ = true;
			break;
		case SDL_MOUSEMOTION:
			if(current_zoom_event.active_)
				current_zoom_event.zoomPoint_ = {test_event.motion.x, test_event.motion.y};
			break;
		case SDL_MOUSEBUTTONUP:
			current_zoom_event.zoomPoint_ = {0 , 0};
			current_zoom_event.active_ = false;
			break;

		default:
			break;
		}
	}
}

std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfHighestDetail(const fd_dim_t& tiling) {
	assert(tiling > 1 && tiling == tkernel.size_);
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
	for (fd_dim_t ty = 0; ty < tiling; ++ty) {
		for (fd_dim_t tx = 0; tx < tiling; ++tx) {
			const fd_coord_t offY = tileH * ty * config.width_;
			const fd_coord_t offX = tileW * tx;

			//iterate over pixels of the tile
			for (fd_coord_t y = 0; y < tileH; ++y) {
				for (fd_coord_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * config.width_)) + (offX + x);
					tile[y * tileW + x] = image[pixIdx];
				}
			}
			fd_float_t weight = tkernel[tx][ty];
			fd_float_t score = measureImageDetail(tile.data(), tileW * tileH) * weight;

			if (score > candidateScore) {
				candidateScore = score;
				candidateTx = tx;
				candidateTy = ty;
			}
		}
	}

	return { (candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2) };
}

bool dive(bool zoom, bool benchmark) {
	fd_float_t detail = measureImageDetail(renderer.imageData_, config.frameSize_);

	if (!benchmark && detail < config.detailThreshold_) {
#ifdef _JAVASCRIPT
//		ThreadPool::getInstance().join();
#endif
		return false;
	}
	if (zoom) {
		process_events();
		std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail;
		if(current_zoom_event.zoomPoint_.first == 0 && current_zoom_event.zoomPoint_.second == 0) {
			centerOfHighDetail = identifyCenterOfTileOfHighestDetail(config.frameTiling_);
			if(camera.panSmoothLength() != config.panSmoothLen_) {
				camera.resetSmoothPan();
				camera.initSmoothPan(0,0, config.panSmoothLen_);
			}
			camera.zoom(centerOfHighDetail.first, centerOfHighDetail.second);
		} else {
			if (current_zoom_event.active_) {
				if(camera.panSmoothLength() != 1) {
					camera.resetSmoothPan();
					camera.initSmoothPan(current_zoom_event.zoomPoint_.first, current_zoom_event.zoomPoint_.second, 1);
				}
				camera.zoom(current_zoom_event.zoomPoint_.first, current_zoom_event.zoomPoint_.second);
			} else {
				camera.zoom(config.width_ / 2.0, config.height_ / 2.0);
			}
		}
	}

	renderer.render();
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

	camera.reset();
	fd_float_t fpsMillis = 1000.0 / config.fps_;
	fd_float_t millisRatio = ((fd_float_t)duration / cnt) / fpsMillis;
#ifndef _FIXEDPOINT
	fd_iter_count_t iterations = std::round((config.startIterations_ / millisRatio)) / 10.0;
#else
	fd_iter_count_t iterations = round((config.startIterations_ / millisRatio)) / 10.0;
#endif

	print(iterations);
#ifdef _BENCHMARK_ONLY
	return true;
#endif
	if (iterations < config.minIterations_)
		config.fps_ = std::max((float)std::floor(config.fps_ * (fd_float_t(iterations) / config.minIterations_)), 1.f);
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
#else
			emscripten_cancel_main_loop();
#endif
	}	else {
		printReport();
	}

	fd_highres_tick_t start = 0;
	while (do_run) {
		start = get_milliseconds();
		tkernel.initAt(config.frameTiling_ / 2, config.frameTiling_ / 2);
		current_zoom_event = ZoomEvent();
		camera.reset();
		camera.initSmoothPan(0,0, config.panSmoothLen_);
		renderer.makeNewPalette();
		renderer.render();

		bool stepResult = true;
		while (do_run && stepResult) {
			stepResult = step();
		}
		print("Duration:", (get_milliseconds() - start) / 1000.0, "seconds");
//		ThreadPool::getInstance().join();
	}

#ifndef _NO_THREADS
	ThreadPool::getInstance().stop();
#endif
	SDL_Quit();
	exit(0);
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
	signal(SIGTERM, sigint_handler);
#endif
#endif
#ifdef _JAVASCRIPT
	emscripten_set_main_loop(run, 0, 1);
#else
	run();
#endif
	return 0;
}
