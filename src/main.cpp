#include <map>
#ifndef _JAVASCRIPT
#include <csignal>
#else
#include <emscripten.h>
#endif

#include "printer.hpp"
#include "threadpool.hpp"
#include "config.hpp"
#include "renderer.hpp"
#include "canvas.hpp"
#include "util.hpp"
#include "imagedetail.hpp"
#include "camera.hpp"

using namespace fractaldive;

bool DO_RUN = true;
Config& CONFIG = Config::getInstance();
Camera CAMERA(CONFIG, CONFIG.zoomFactor_);
Renderer RENDERER(CONFIG, CAMERA,CONFIG.startIterations_);
Canvas CANVAS(CONFIG.width_, CONFIG.height_, false);

struct ZoomEvent {
	std::pair<size_t, size_t> zoomPoint_ = { 0, 0};
	bool active_ = false;
};

ZoomEvent current_zoom_event;
void process_events() {
	SDL_Event test_event;
	while (SDL_PollEvent(&test_event)) {
		switch (test_event.type) {
		case SDL_QUIT:
			DO_RUN = false;
			break;
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

std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfDetail(const fd_dim_t& tiling) {
	assert(tiling > 1);
	const fd_coord_t tileW = std::floor(fd_float_t(CONFIG.width_) / fd_float_t(tiling));
	const fd_coord_t tileH = std::floor(fd_float_t(CONFIG.height_) / fd_float_t(tiling));
	assert(tileW > 1);
	assert(tileH > 1);

	const size_t tileSize = tileW * tileH;

	const auto& image = RENDERER.imageData_;
	std::vector<fd_image_pix_t> tile(tileSize);
	std::map<fd_float_t,std::pair<fd_float_t, fd_float_t>> candidates;


	//iterate over tiles
	for (fd_dim_t ty = 0; ty < tiling; ++ty) {
		for (fd_dim_t tx = 0; tx < tiling; ++tx) {
			const fd_coord_t offY = tileH * ty * CONFIG.width_;
			const fd_coord_t offX = tileW * tx;

			//iterate over pixels of the tile
			for (fd_coord_t y = 0; y < tileH; ++y) {
				for (fd_coord_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * CONFIG.width_)) + (offX + x);
					tile[y * tileW + x] = image[pixIdx];
				}
			}
			fd_float_t score = measureImageDetail(tile.data(), tileW * tileH);
			candidates[score] = {tx, ty};
		}
	}

	fd_float_t min = (*candidates.begin()).first;
	fd_float_t max = (*candidates.rbegin()).first;
	fd_float_t med = (max - min) * 0.75;
	fd_float_t candidateScore = max + 1;
	fd_coord_t candidateTx = 0;
	fd_coord_t candidateTy = 0;

	for(auto& p : candidates) {
		if(std::fabs(p.first - med) < std::fabs(candidateScore - med)) {
			candidateScore = p.first;
			candidateTx = p.second.first;
			candidateTy = p.second.second;
		}
	}

	return { (candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2) };
}

bool dive(bool zoom, bool benchmark) {
	fd_float_t detail = measureImageDetail(RENDERER.imageData_, CONFIG.frameSize_);

	if (!benchmark && detail < CONFIG.detailThreshold_) {
		return false;
	}
	if (zoom) {
		process_events();
		std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail;
		if(current_zoom_event.zoomPoint_.first == 0 && current_zoom_event.zoomPoint_.second == 0) {
			centerOfHighDetail = identifyCenterOfTileOfDetail(CONFIG.frameTiling_);
			if(CAMERA.panSmoothLength() != CONFIG.panSmoothLen_) {
				CAMERA.resetSmoothPan();
				CAMERA.initSmoothPan(0,0, CONFIG.panSmoothLen_);
			}
			CAMERA.zoom(centerOfHighDetail.first, centerOfHighDetail.second);
		} else {
			if (current_zoom_event.active_) {
				if(CAMERA.panSmoothLength() != 1) {
					CAMERA.resetSmoothPan();
					CAMERA.initSmoothPan(current_zoom_event.zoomPoint_.first, current_zoom_event.zoomPoint_.second, 1);
				}
				CAMERA.zoom(current_zoom_event.zoomPoint_.first, current_zoom_event.zoomPoint_.second);
			} else {
				CAMERA.zoom(CONFIG.width_ / 2.0, CONFIG.height_ / 2.0);
			}
		}
	}

#ifndef _NO_THREADS
	if(ThreadPool::getInstance().taskCount() > 0) {
		ThreadPool::getInstance().join();
	}
#endif

	CANVAS.draw(RENDERER.imageData_);
	RENDERER.render();
	return true;
}

bool auto_scale_max_iterations() {
	auto start = get_milliseconds();
	auto duration = start;

	size_t cnt = 0;
	while ((duration = (get_milliseconds() - start)) < CONFIG.benchmarkTimeoutMillis_) {
		dive(false, true);
		++cnt;
	}

	CAMERA.reset();
	fd_float_t fpsMillis = 1000.0 / CONFIG.fps_;
	fd_float_t millisRatio = ((fd_float_t)duration / cnt) / fpsMillis;
	fd_iter_count_t iterations = round((CONFIG.startIterations_ / millisRatio)) / 10.0;

	print(iterations);
#ifdef _BENCHMARK_ONLY
	return true;
#endif
	if (iterations < CONFIG.minIterations_)
		CONFIG.fps_ = std::max((float)std::floor(CONFIG.fps_ * (fd_float_t(iterations) / CONFIG.minIterations_)), 1.f);
	iterations = std::min(CONFIG.maxIterations_, std::max(iterations, CONFIG.minIterations_));
	RENDERER.setMaxIterations(iterations);
	return false;
}

bool step() {
	auto start = get_milliseconds();
	bool result = dive(true, false);
	auto duration = get_milliseconds() - start;

	int32_t targetMillis = 1000.0 / CONFIG.fps_;
	int32_t diff = targetMillis - duration;

	if (result) {
		if (diff > 0) {
			sleep_millis(diff);
		} else if (diff < 0)
			printErr("Underrun: ", std::abs(diff));
	}
	return result;
}

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
	print(pad_string("FPS:", padWidth), CONFIG.fps_);
	print(pad_string("Max iterations:", padWidth), RENDERER.getMaxIterations(), "of", CONFIG.maxIterations_);
	print(pad_string("Detail threshold:", padWidth), CONFIG.detailThreshold_);
	print(pad_string("Pan history:", padWidth), CONFIG.panSmoothLen_);
	print("#####");
	print("");
}

void run() {
	if(auto_scale_max_iterations()){
			DO_RUN = false;
#ifndef _JAVASCRIPT
			ThreadPool::getInstance().stop();
#else
			emscripten_cancel_main_loop();
#endif
	}	else {
		printReport();
	}

	fd_highres_tick_t start = 0;
	while (DO_RUN) {
		start = get_milliseconds();
		current_zoom_event = ZoomEvent();
		CAMERA.reset();
		CAMERA.initSmoothPan(0,0, CONFIG.panSmoothLen_);
		RENDERER.makeNewPalette();
		RENDERER.render();

		bool stepResult = true;
		while (DO_RUN && stepResult) {
			stepResult = step();
		}
		print("Duration:", (get_milliseconds() - start) / 1000.0, "seconds");
	}

	ThreadPool::getInstance().stop();
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
	DO_RUN = false;
}
#endif
#endif

int main() {
	assert(CONFIG.width_ >= 16 && CONFIG.width_ % 2 == 0);
	assert(CONFIG.height_ >= 16 && CONFIG.height_ % 2 == 0);
	assert(CONFIG.startIterations_ > 3);
	assert(CONFIG.fps_ > 0);

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
