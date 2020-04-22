#include <cstdint>
#include <cstddef>
#include <set>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <chrono>
#include <thread>

#ifndef _JAVASCRIPT
#include <csignal>
#else
#include <emscripten.h>
#endif

#include "types.hpp"

using namespace fractaldive;

#include "renderer.hpp"
#include "canvas.hpp"

constexpr dim_t WIDTH = 256;
constexpr dim_t HEIGHT = 256;
constexpr dim_t FRAME_SIZE = WIDTH * HEIGHT;

#ifdef _JAVASCRIPT

#ifdef _JAVASCRIPT_MT
dim_t max_iterations = 400;
#else
dim_t max_iterations = 200;
#endif

#else
dim_t max_iterations = 500;
#endif



fractaldive::Renderer renderer(WIDTH, HEIGHT, max_iterations);
fractaldive::Canvas canvas(WIDTH, HEIGHT, false);
bool do_run = true;

float_t entropy(const grey_image_t& greyImage, const size_t& size) {
	std::map<uint8_t, int32_t> frequencies;

	for (size_t i = 0; i < size; ++i)
    ++frequencies[greyImage[i]];

  float_t infocontent = 0 ;
  for (const auto& p : frequencies ) {
     float_t freq = static_cast<float_t>( p.second ) / size;
     infocontent -= freq * log2( freq ) ;
  }

  return infocontent;
}

float_t numberOfZeroes(const grey_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	float_t zeroes = 0;
	for (size_t i = 0; i < size; i++) {
			if(greyImage[i] == 0)
				++zeroes;
	}
	return zeroes / size;
}

float_t numberOfColors(const grey_image_t& greyImage, const size_t& size) {
	assert(size > 0);
	std::set<uint8_t> clrs;
	for (size_t i = 0; i < size; i++) {
			clrs.insert(greyImage[i]);
	}
	return (clrs.size() / 255.0);
}

float_t measureDetail(const grey_image_t& greyImage, const size_t& size) {
//	return measureEntropy(greyImage,size);
	return (numberOfColors(greyImage, size) + ((1.0 - numberOfZeroes(greyImage, size)) * 3)) / 4.0;
}

std::pair<coord_t, coord_t> identifyCenterOfTileOfHighestDetail(const dim_t& numTilesX, const dim_t& numTilesY) {
	const dim_t tileW = std::floor(float_t(renderer.WIDTH_) / numTilesX);
	const dim_t tileH = std::floor(float_t(renderer.HEIGHT_) / numTilesY);
	assert(tileW > 1);
	assert(tileH > 1);

	const grey_image_t& greyImage = renderer.greydata_;
	ccomp_t tile[FRAME_SIZE];

	std::vector<float_t> tileScores;
	float_t candidateScore = 0;
	dim_t candidateTx = 0;
	dim_t candidateTy = 0;

	//iterate over tiles
	for(dim_t ty = 0; ty < numTilesY; ++ty) {
		for(dim_t tx = 0; tx < numTilesX; ++tx) {
			const dim_t offY = tileH * ty * renderer.WIDTH_;
			const dim_t offX = tileW * tx;

			//iterate over pixels of the tile
			for(dim_t y = 0; y < tileH; ++y) {
				for(dim_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * renderer.WIDTH_)) + (offX + x);
					tile[y * tileW + x] = greyImage[pixIdx];
				}
			}

			float_t score = measureDetail(tile, tileW * tileH);
			if(score > candidateScore) {
				candidateScore = score;
				candidateTx = tx;
				candidateTy = ty;
			}

		}
	}
	//std::cerr << "cand: " << candidateTx << ":" << candidateTy << std::endl;
	return {(candidateTx * tileW) + (tileW / 2), (candidateTy * tileH) + (tileH / 2)};
}

bool dive() {
	float_t detail = measureDetail(renderer.greydata_, renderer.WIDTH_ * renderer.HEIGHT_);
//	std::cerr << "detail:" << detail << " " << "zoom:" << renderer.getZoom() << std::endl;
	if(detail < 0.1) {
#ifdef _JAVASCRIPT
		emscripten_cancel_main_loop();
#endif
		return false;
	}
	std::pair<coord_t,coord_t> centerOfHighDetail = identifyCenterOfTileOfHighestDetail(5, 5);
	coord_t hDiff = centerOfHighDetail.first - (renderer.WIDTH_ / 2);
	coord_t vDiff = centerOfHighDetail.second - (renderer.HEIGHT_ / 2);

	renderer.pan(hDiff / 20, vDiff / 20);
  renderer.zoomAt(renderer.WIDTH_ / 2, renderer.HEIGHT_ / 2, 1.05, true);
	renderer.render();
	canvas.draw(renderer.rgbdata_);
	return true;
}

void auto_benchmark() {
	fractaldive::Renderer bmRenderer(WIDTH, HEIGHT, max_iterations);
	bmRenderer.zoomAt(WIDTH/2, HEIGHT/2, 3, true);
	auto start = std::chrono::system_clock::now();
	for(size_t i = 0; i < 10; ++i)
		bmRenderer.render();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now() - start);

	max_iterations = std::round(50000.0  / std::pow(std::ceil(float_t(duration.count()) / 200.0), 3.0));
	renderer.setMaxIterations(max_iterations);
	std::cerr << "benchmark: " << duration.count() << " = " << max_iterations << std::endl;
}

bool step() {
#ifndef _JAVASCRIPT
		auto start = std::chrono::system_clock::now();
#endif

		bool result = dive();

#ifndef _JAVASCRIPT
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - start);

		std::cerr << 1000.0 / duration.count() << std::endl;
#endif
		return result;
}

#ifdef _JAVASCRIPT
void js_step() {
	step();
}
#endif

void run() {
	auto_benchmark();

  while(do_run) {
  	renderer.pan((rand() % 10) - 20, (rand() % 10) - 20);
  	renderer.render();

  	#ifdef _JAVASCRIPT
  	emscripten_set_main_loop(js_step, 0, 1);
#else
		while(do_run && step()) {
		}
  }

	SDL_Quit();
#endif
}

#ifndef _JAVASCRIPT
void sigint_handler(int sig) {
	do_run = false;
}
#endif

int main() {
	srand (time(NULL));
#ifndef _JAVASCRIPT
	signal(SIGINT, sigint_handler);
#endif
	run();
	return 0;
}
