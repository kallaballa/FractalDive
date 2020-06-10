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
#ifndef _AMIGA
#include <chrono>
typedef std::chrono::high_resolution_clock highres_clock;
#endif

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
#include "types.hpp"
#include "renderer.hpp"
#include "canvas.hpp"
#include "timetracker.hpp"


using namespace fractaldive;
#ifndef _AMIGA
constexpr fd_dim_t WIDTH = 256;
constexpr fd_dim_t HEIGHT = 256;
#else
constexpr fd_dim_t WIDTH = 30;
constexpr fd_dim_t HEIGHT = 30;
#endif

#ifndef _AMIGA
constexpr size_t FPS = 24;
#else
constexpr size_t FPS = 6;
#endif

constexpr fd_dim_t FRAME_SIZE = WIDTH * HEIGHT;

fd_iter_count_t max_iterations = 100;

fractaldive::Renderer renderer(WIDTH, HEIGHT, max_iterations);
fractaldive::Canvas canvas(WIDTH, HEIGHT, false);
bool do_run = true;


inline uint32_t get_milliseconds() {
	return SDL_GetTicks();
}

#ifndef _AMIGA
inline uint64_t get_microseconds() {
	return std::chrono::duration_cast<std::chrono::microseconds>(highres_clock::now().time_since_epoch()).count();
}
#else
inline uint64_t get_microseconds() {
	assert(false);
	return 0;
}
#endif

inline void sleep_millis(uint32_t millis) {
#ifdef _JAVASCRIPT
		emscripten_sleep(millis);
#else
		SDL_Delay(millis);
#endif
}

fd_float_t entropy(const shadow_image_t& greyImage, const size_t& size) {
	std::map<uint8_t, int32_t> frequencies;

	for (size_t i = 0; i < size; ++i)
		++frequencies[greyImage[i]];

	fd_float_t infocontent = 0;
	for (const auto& p : frequencies) {
		fd_float_t freq = static_cast<fd_float_t>(p.second) / size;
		infocontent -= freq * log2(freq);
	}

	return infocontent;
}

fd_float_t numberOfZeroes(const shadow_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	fd_float_t zeroes = 0;
	for (size_t i = 0; i < size; i++) {
		if (greyImage[i] == 0)
			++zeroes;
	}
	return zeroes / size;
}

fd_float_t numberOfColors(const shadow_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	std::set<fd_shadow_comp_t> clrs;
	for (size_t i = 0; i < size; i++) {
		clrs.insert(greyImage[i]);
	}
	return (fd_float_t(clrs.size()) / size);
}

fd_float_t numberOfChanges(const shadow_image_t& greyImage, const size_t& size) {
	size_t numChanges = 0;
	fd_shadow_comp_t last = 0;

	for (size_t i = 0; i < size; i++) {
		if(last != greyImage[i])
			++numChanges;
		last = greyImage[i];
	}
	return (fd_float_t(numChanges) / size);
}

fd_float_t measureDetail(const shadow_image_t& greyImage, const size_t& size) {
//identifies areas of interest better at a low resolution but is significantly more costly
//return entropy(greyImage,size + ((1.0 - numberOfZeroes(greyImage, size)) * 2)) / 3.0;
	return numberOfChanges(greyImage, size);
}

std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfHighestDetail(const fd_dim_t& numTilesX,
		const fd_dim_t& numTilesY) {
	const fd_dim_t tileW = std::floor(fd_float_t(renderer.WIDTH_) / numTilesX);
	const fd_dim_t tileH = std::floor(fd_float_t(renderer.HEIGHT_) / numTilesY);
	assert(tileW > 1);
	assert(tileH > 1);

	const auto& shadowImage = renderer.shadowdata_;
	fd_shadow_comp_t tile[FRAME_SIZE];

	std::vector<fd_float_t> tileScores;
	fd_float_t candidateScore = 0;
	fd_dim_t candidateTx = 0;
	fd_dim_t candidateTy = 0;

	//iterate over tiles
	for (fd_dim_t ty = 0; ty < numTilesY; ++ty) {
		for (fd_dim_t tx = 0; tx < numTilesX; ++tx) {
			const fd_dim_t offY = tileH * ty * renderer.WIDTH_;
			const fd_dim_t offX = tileW * tx;

			//iterate over pixels of the tile
			for (fd_dim_t y = 0; y < tileH; ++y) {
				for (fd_dim_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * renderer.WIDTH_)) + (offX + x);
					tile[y * tileW + x] = shadowImage[pixIdx];
				}
			}

			fd_float_t score = measureDetail(tile, tileW * tileH);
			if (score > candidateScore) {
				candidateScore = score;
				candidateTx = tx;
				candidateTy = ty;
			}

		}
	}

	candidateTx = rand() % 2 ? rand() % numTilesX : candidateTx;
	candidateTy = rand() % 2 ? rand() % numTilesY : candidateTy;
	return {(candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2)};
}

bool dive(bool zoom, bool benchmark) {
	TimeTracker& tt = TimeTracker::getInstance();
	fd_float_t detail;
	tt.execute("measureDetail", [&](){
		detail = measureDetail(renderer.shadowdata_, renderer.WIDTH_ * renderer.HEIGHT_);
	});
	if (!benchmark && detail < 0.0001) {
#ifdef _JAVASCRIPT
		renderer.reset();
		renderer.render();
//		emscripten_cancel_main_loop();
#endif
		return false;
	}
	std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail;
	tt.execute("centerofHighDetail", [&](){
#ifndef _AMIGA
		centerOfHighDetail = identifyCenterOfTileOfHighestDetail(3, 3);
#else
		centerOfHighDetail = identifyCenterOfTileOfHighestDetail(3, 3);
#endif
	});

	fd_coord_t hDiff = centerOfHighDetail.first - (renderer.WIDTH_ / 2);
	fd_coord_t vDiff = centerOfHighDetail.second - (renderer.HEIGHT_ / 2);

	if (zoom) {
		renderer.pan(hDiff / 20, vDiff / 20);
		fd_float_t zf = 0.60 / FPS;
		renderer.zoomAt(renderer.WIDTH_ / 2, renderer.HEIGHT_ / 2, 1.0 + zf, true);
	}
	tt.execute("render", [&](){
		renderer.render();
	});

	tt.execute("draw", [&](){
		canvas.draw(renderer.imgdata_);
	});

	return true;
}
//automatic benchmark and
void auto_scale_max_iterations() {
	auto start = get_milliseconds();
#ifndef _AMIGA
	fd_float_t prescale = 1.0;
#else
	fd_float_t prescale = 0.02;
#endif

#ifndef _NO_TIMETRACK
	prescale /= 10.0;
#endif

	fd_float_t postscale = 1.0 / prescale;

	for (size_t i = 0; i < std::ceil(100.0 * prescale); ++i) {
		dive(false,true);
	}

	auto duration = get_milliseconds() - start;
	renderer.reset();

	fd_float_t fpsMillis = 1000.0 / FPS;
	fd_float_t millisRatio = (pow(duration * postscale, 1.20) / fpsMillis);
#ifndef _FIXEDPOINT
	fd_iter_count_t iterations = (max_iterations / millisRatio) * 50.0;
#else
	fd_iter_count_t iterations = (max_iterations.ToFloat() / millisRatio) * 50.0;
#endif

#ifdef _JAVASCRIPT_MT
		iterations = (iterations * (ThreadPool::cores() - 1));
#endif

#ifndef _FIXEDPOINT
	renderer.setMaxIterations(fmax(round(iterations), 3.0));
#else
	renderer.setMaxIterations(fmax(round(iterations.ToFloat()), 3.0));
#endif
}

bool step() {
	auto start = get_milliseconds();
	bool result = dive(true,false);
	auto duration = get_milliseconds() - start;

	int32_t targetMillis = 1000.0 / FPS;
	int32_t diff = targetMillis - duration;

	if (diff > 0) {
		sleep_millis(diff);
	} else if (diff < 0)
		printErr("Underrun: ", std::abs(diff));

	return result;
}

#ifdef _JAVASCRIPT
void js_step() {
	step();
#ifndef _NO_TIMETRACK
			printErr(tt.str());
			tt.epoch();
#endif
}
#endif

void printInfo() {
	print("Threads:", ThreadPool::cores());
#ifdef _AUTOVECTOR
  print("Auto Vector/SIMD: on");
#else
  print("Auto Vector/SIMD: off");
#endif

#ifdef _FIXEDPOINT
  print("Arithmetic: fixed point");
#else
  print("Arithmetic: floating point");
#endif
	print("Max iterations:", renderer.getMaxIterations());
}

void run() {
	auto_scale_max_iterations();
	printInfo();

#ifndef _NO_TIMETRACK
	TimeTracker& tt = TimeTracker::getInstance();
#endif

	while (do_run) {
		renderer.pan((rand() % WIDTH / 16) - (WIDTH / 8), (rand() % WIDTH / 16) - (WIDTH / 8));
		renderer.render();

#ifdef _JAVASCRIPT
		emscripten_set_main_loop(js_step, 0, 1);
#else
		bool stepResult = true;
		while (do_run && stepResult) {
#ifndef _NO_TIMETRACK
			tt.execute("step", [&](){
				stepResult = step();
			});
			printErr(tt.str());
			tt.epoch();
#else
			stepResult = step();
#endif
		}
#endif
		renderer.reset();
	}
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
	assert(WIDTH >= 16 && is_power_of_two(WIDTH));
	assert(HEIGHT >= 16 && is_power_of_two(HEIGHT));
	assert(max_iterations > 3);
	assert(FPS > 0);

	srand(time(NULL));
#ifndef _JAVASCRIPT
#ifndef _AMIGA
	signal(SIGINT, sigint_handler);
#endif
#endif
	run();
	return 0;
}
