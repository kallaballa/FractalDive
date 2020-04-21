#include <cstdint>
#include <cstddef>
#include <set>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>

#ifndef _JAVASCRIPT
#include <chrono>
#include <csignal>
#include <thread>
#else
#include <emscripten.h>
#endif


#include "renderer.hpp"
#include "canvas.hpp"

constexpr size_t WIDTH = 256;
constexpr size_t HEIGHT = 256;
size_t MAX_ITERATIONS = 250;

namespace fd = fractaldive;

fd::Renderer renderer(WIDTH, HEIGHT, MAX_ITERATIONS);
fd::Canvas canvas(WIDTH, HEIGHT, false);
bool do_run = true;

double entropy(std::vector<uint8_t> greyImage) {
	std::map<uint8_t, int32_t> frequencies;

	for (const auto& c : greyImage)
    ++frequencies[c];

	size_t size = greyImage.size();
  double infocontent = 0 ;
  for (const auto& p : frequencies ) {
     double freq = static_cast<double>( p.second ) / size;
     infocontent -= freq * log2( freq ) ;
  }

  return infocontent;
}

double numberOfZeroes(const uint8_t* greyImage, const size_t& size) {
	assert(size > 0);
	double zeroes = 0;
	for (size_t i = 0; i < size; i++) {
			if(greyImage[i] == 0)
				++zeroes;
	}
	return 1.0 - (zeroes / size);
}

double numberOfColors(const uint8_t* greyImage, const size_t& size) {
	assert(size > 0);
	std::set<uint8_t> clrs;
	for (size_t i = 0; i < size; i++) {
			clrs.insert(greyImage[i]);
	}
	return (clrs.size() / 255.0);
}

double measureDetail(const uint8_t* greyImage, const size_t& size) {
//	return measureEntropy(greyImage);
	return (numberOfColors(greyImage, size) + numberOfZeroes(greyImage, size)) / 2.0;
}

std::pair<int32_t, int32_t> identifyCenterOfTileOfHighestDetail(const size_t& numTilesX, const size_t& numTilesY) {
	const size_t tileW = std::floor(float(renderer.WIDTH_) / numTilesX);
	const size_t tileH = std::floor(float(renderer.HEIGHT_) / numTilesY);
	assert(tileW > 1);
	assert(tileH > 1);

	const uint8_t* greyImage = renderer.greydata_;
	uint8_t* tile = new uint8_t[tileW * tileH];
	std::vector<double> tileScores;
	double candidateScore = 0;
	int32_t candidateTx = 0;
	int32_t candidateTy = 0;
	//iterate over tiles
	for(size_t ty = 0; ty < numTilesY; ++ty) {
		for(size_t tx = 0; tx < numTilesX; ++tx) {
			const size_t offY = tileH * ty * renderer.WIDTH_;
			const size_t offX = tileW * tx;

			//iterate over pixels of the tile
			for(size_t y = 0; y < tileH; ++y) {
				for(size_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * renderer.WIDTH_)) + (offX + x);
					tile[y * tileW + x] = greyImage[pixIdx];
				}
			}

			double score = measureDetail(tile, tileW * tileH);
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
	double detail = measureDetail(renderer.greydata_, renderer.WIDTH_ * renderer.HEIGHT_);
	if(detail < 0.1) {
#ifdef _JAVASCRIPT
		emscripten_cancel_main_loop();
#endif
		return false;
	}
	std::pair<int32_t,int32_t> centerOfHighDetail = identifyCenterOfTileOfHighestDetail(5, 5);
	int32_t hDiff = centerOfHighDetail.first - (renderer.WIDTH_ / 2);
	int32_t vDiff = centerOfHighDetail.second - (renderer.HEIGHT_ / 2);

	renderer.pan(hDiff / 20, vDiff / 20);
  renderer.zoomAt(renderer.WIDTH_ / 2, renderer.HEIGHT_ / 2, 1.05, true);
	renderer.render();
	canvas.draw(renderer.rgbdata_);
	return true;
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
	renderer.pan((rand() % 10) - 20, (rand() % 10) - 20);
	renderer.render();

#ifdef _JAVASCRIPT
	emscripten_set_main_loop(js_step, 0, 1);
#else
	while(do_run && step()) {
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
