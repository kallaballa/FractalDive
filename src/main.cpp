#include <cstdint>
#include <cstddef>
#include <set>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <chrono>
#include <complex>
#include "threadpool.hpp"

#ifndef _JAVASCRIPT
#include <csignal>
#else
#include <emscripten.h>
#endif

#include "types.hpp"

using namespace fractaldive;

#include "renderer.hpp"
#include "canvas.hpp"

constexpr fd_dim_t WIDTH = 200;
constexpr fd_dim_t HEIGHT = 200;
constexpr fd_dim_t FRAME_SIZE = WIDTH * HEIGHT;
constexpr size_t FPS = 12;

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
	return numberOfColors(greyImage, size);
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

bool dive(bool zoom) {
	fd_float_t detail = measureDetail(renderer.greydata_, renderer.WIDTH_ * renderer.HEIGHT_);

	if (detail < 0.05) {
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
	auto start = std::chrono::system_clock::now();

	fd_mandelfloat_t zr = 0.0, zi = 0.0;
	fd_mandelfloat_t cr = 0.5;
	fd_mandelfloat_t ci = 0.5;
	fd_mandelfloat_t two = 2.0;
	for (size_t i = 0; i < 6000000; ++i) {
		const fd_mandelfloat_t& temp = zr * zr - zi * zi + cr;
		zi = two * zr * zi + ci;
		zr = temp;
	}

	//hacky way of convincing the compiler to not optimize that benchmark loop away,
	//without actually doing something with it
	__asm__ __volatile__("" :: "m" (zr));

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

	fd_float_t extra_cores = ThreadPool::extra_cores();
	fd_float_t millis = duration.count();
	fd_float_t divider;
	if (extra_cores > 0)
		divider = (millis / (extra_cores + 1)) + ((millis / 100.0) * (ThreadPool::extra_cores() + 1));
	else
		divider = millis;

	fd_float_t iterations = ((WIDTH * HEIGHT / 13.0) / divider);
	renderer.setMaxIterations(std::round(iterations));
}

bool step() {
	auto start = std::chrono::system_clock::now();
	bool result = dive(true);
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

	int32_t targetMillis = 1000.0 / FPS;
	int32_t diff = targetMillis - duration.count();

	if (diff > 0) {
#ifndef _JAVASCRIPT
		std::this_thread::sleep_for(std::chrono::milliseconds(diff));
#else
		emscripten_sleep(diff);
#endif
	} else if (diff < 0)
		std::cerr << "Underrun: " << std::abs(diff) << std::endl;

	return result;
}

#ifdef _JAVASCRIPT
void js_step() {
	step();
}
#endif

void run() {
	auto_scale_max_iterations();

  std::cout << "Threads:" << ThreadPool::extra_cores() + 1 << std::endl;
#ifdef _AUTOVECTOR
  std::cout << "Auto Vector/SIMD: on" << std::endl;
#else
  std::cout << "Auto Vector/SIMD: off" << std::endl;
#endif

#ifdef _FIXEDPOINT
  std::cout << "Arithmetic: fixed point" <<  std::endl;
#else
  std::cout << "Arithmetic: floating point" <<  std::endl;
#endif
	std::cout << "Max iterations:" << renderer.getMaxIterations() << std::endl;

	while (do_run) {
		renderer.pan((rand() % 10) - 20, (rand() % 10) - 20);
		renderer.render();

#ifdef _JAVASCRIPT
		emscripten_set_main_loop(js_step, 0, 1);
#else
		while (do_run && step()) {
		}
#endif
		renderer.reset();
	}
	SDL_Quit();
}

#ifndef _JAVASCRIPT
void sigint_handler(int sig) {
	do_run = false;
}
#endif

int main() {
	srand(time(NULL));
#ifndef _JAVASCRIPT
	signal(SIGINT, sigint_handler);
#endif
	run();
	return 0;
}
