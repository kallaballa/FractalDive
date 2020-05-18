
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

#include <chrono>

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

using namespace fractaldive;
#ifndef _AMIGA
constexpr fd_dim_t WIDTH = 200;
constexpr fd_dim_t HEIGHT = 200;
#else
constexpr fd_dim_t WIDTH = 50;
constexpr fd_dim_t HEIGHT = 50;
#endif

#ifndef _AMIGA
constexpr size_t FPS = 12;
#else
constexpr size_t FPS = 6;
#endif

constexpr fd_dim_t FRAME_SIZE = WIDTH * HEIGHT;

fd_dim_t max_iterations = 100;

fractaldive::Renderer renderer(WIDTH, HEIGHT, max_iterations);
fractaldive::Canvas canvas(WIDTH, HEIGHT, false);
bool do_run = true;

fd_float_t entropy(const grey_image_t& greyImage, const size_t& size) {
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

fd_float_t numberOfZeroes(const grey_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	fd_float_t zeroes = 0;
	for (size_t i = 0; i < size; i++) {
		if (greyImage[i] == 0)
			++zeroes;
	}
	return zeroes / size;
}

fd_float_t numberOfColors(const grey_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	std::set<uint8_t> clrs;
	for (size_t i = 0; i < size; i++) {
		clrs.insert(greyImage[i]);
	}
	return (clrs.size() / 255.0);
}

fd_float_t measureDetail(const grey_image_t& greyImage, const size_t& size) {
	//return entropy(greyImage,size + ((1.0 - numberOfZeroes(greyImage, size)) * 2)) / 3.0;
	return (numberOfColors(greyImage, size)  + ((1.0 - numberOfZeroes(greyImage, size)) * 2)) / 3.0;;
}

std::pair<fd_coord_t, fd_coord_t> identifyCenterOfTileOfHighestDetail(const fd_dim_t& numTilesX,
		const fd_dim_t& numTilesY) {
	const fd_dim_t tileW = std::floor(fd_float_t(renderer.WIDTH_) / numTilesX);
	const fd_dim_t tileH = std::floor(fd_float_t(renderer.HEIGHT_) / numTilesY);
	assert(tileW > 1);
	assert(tileH > 1);

	const grey_image_t& greyImage = renderer.greydata_;
	fd_ccomp_t tile[FRAME_SIZE];

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
					tile[y * tileW + x] = greyImage[pixIdx];
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
	return {(candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2)};
}

bool dive(bool zoom, bool benchmark) {
	fd_float_t detail = measureDetail(renderer.greydata_, renderer.WIDTH_ * renderer.HEIGHT_);

	if (!benchmark && detail < 0.05) {
#ifdef _JAVASCRIPT
		renderer.reset();
		renderer.render();
//		emscripten_cancel_main_loop();
#endif
		return false;
	}
	std::pair<fd_coord_t, fd_coord_t> centerOfHighDetail = identifyCenterOfTileOfHighestDetail(5, 5);
	fd_coord_t hDiff = centerOfHighDetail.first - (renderer.WIDTH_ / 2);
	fd_coord_t vDiff = centerOfHighDetail.second - (renderer.HEIGHT_ / 2);

	if (zoom) {
		renderer.pan(hDiff / 20, vDiff / 20);
		renderer.zoomAt(renderer.WIDTH_ / 2, renderer.HEIGHT_ / 2, 1.05, true);
	}
	renderer.render();
	canvas.draw(renderer.rgbdata_);
	return true;
}

void auto_scale_max_iterations() {
	Printer& printer = Printer::getInstance();
	auto start = std::chrono::system_clock::now();
#ifndef _AMIGA
	fd_float_t prescale = 1.0;
	fd_float_t postscale = 1.0;
#else
	fd_float_t prescale = 0.1;
	fd_float_t postscale = 7;
#endif

	for (size_t i = 0; i < std::ceil(100.0 * prescale); ++i) {
		dive(false,true);
	}

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	renderer.reset();

	printer.printErr(duration.count());
	fd_float_t fpsMillis = 1000.0 / FPS;
	fd_float_t millisRatio = (pow(duration.count() * postscale, 1.20) / fpsMillis);
	fd_float_t iterations = (max_iterations / millisRatio) * 55.0;
#ifdef _JAVASCRIPT_MT
	if(ThreadPool::extra_cores() > 1)
		iterations = (iterations * ThreadPool::extra_cores()) / 2.5;
#endif
	renderer.setMaxIterations(round(iterations));
}

bool step() {
	Printer& printer = Printer::getInstance();
	auto start = std::chrono::system_clock::now();
	bool result = dive(true,false);
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	int32_t targetMillis = 1000.0 / FPS;
	int32_t diff = targetMillis - duration.count();

	if (diff > 0) {
#ifndef _JAVASCRIPT
#ifndef _AMIGA
#ifndef _NO_THREADS
		std::this_thread::sleep_for(std::chrono::milliseconds(diff));
#else
		usleep(1000.0 / FPS);
#endif
#endif
#endif
	} else if (diff < 0)
		printer.printErr("Underrun: ", std::abs(diff));

	return result;
}

#ifdef _JAVASCRIPT
void js_step() {
	step();
}
#endif

void run() {
	auto_scale_max_iterations();
	Printer& printer = Printer::getInstance();
	printer.print("Threads:", ThreadPool::extra_cores() + 1);
#ifdef _AUTOVECTOR
  printer.print("Auto Vector/SIMD: on");
#else
  printer.print("Auto Vector/SIMD: off");
#endif

#ifdef _FIXEDPOINT
  printer.print("Arithmetic: fixed point");
#else
  printer.print("Arithmetic: floating point");
#endif
	printer.print("Max iterations:", renderer.getMaxIterations());

	while (do_run) {
		renderer.pan((rand() % 10) - 20, (rand() % 10) - 20);
		renderer.render();

#ifdef _JAVASCRIPT
		emscripten_set_main_loop(js_step, FPS, 1);
#else
		while (do_run && step()) {
		}
#endif
		renderer.reset();
	}
	SDL_Quit();
}

#ifndef _JAVASCRIPT
#ifndef _AMIGA
void sigint_handler(int sig) {
	do_run = false;
}
#endif
#endif

int main() {
	srand(time(NULL));
#ifndef _JAVASCRIPT
#ifndef _AMIGA
	signal(SIGINT, sigint_handler);
#endif
#endif
	run();
	return 0;
}
