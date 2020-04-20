#include <cstdint>
#include <cstddef>
#include <set>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <csignal>
#include <thread>
#include <map>

#include "renderer.hpp"
#include "canvas.hpp"

constexpr size_t WIDTH = 256;
constexpr size_t HEIGHT = 256;
constexpr size_t MAX_ITERATIONS = 250;
bool do_run = true;

namespace fd = fractaldive;

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

double numberOfColors(const std::vector<uint8_t> greyImage) {
	std::set<uint8_t> clrs;
	double zeroes = 0;
	for (size_t i = 0; i < greyImage.size(); i++) {
		if (greyImage[i] == 0)
			zeroes++;
		else
			clrs.insert(greyImage[i]);
	}

	return ((clrs.size() / 255.0) + (1.0 - (zeroes / greyImage.size()))) / 2.0;
}

double measureDetail(const std::vector<uint8_t> greyImage) {
//	return measureEntropy(greyImage);
	return numberOfColors(greyImage);
}

std::pair<int32_t, int32_t> identifyCenterOfTileOfHighestDetail(const size_t& numTilesX, const size_t& numTilesY, fd::Renderer& renderer, fd::Canvas& canvas) {
	const size_t tileW = std::floor(float(renderer.WIDTH_) / numTilesX);
	const size_t tileH = std::floor(float(renderer.HEIGHT_) / numTilesY);
	assert(tileW > 1);
	assert(tileH > 1);

	const std::vector<uint8_t>& greyImage = renderer.greydata_;
	std::vector<uint8_t> tile(tileW * tileH);
	std::vector<double> tileScores;
	double candidateScore = 0;
	int32_t candidateTx = 0;
	int32_t candidateTy = 0;
	//iterate over tiles
	for(size_t ty = 0; ty < numTilesY; ++ty) {
		for(size_t tx = 0; tx < numTilesX; ++tx) {
			const size_t offY = tileH * ty * renderer.WIDTH_;
			const size_t offX = tileW * tx;

			tile.clear();
			//iterate over pixels of the tile
			for(size_t y = 0; y < tileH; ++y) {
				for(size_t x = 0; x < tileW; ++x) {
					const size_t pixIdx = (offY + (y * renderer.WIDTH_)) + (offX + x);
					tile.push_back(greyImage[pixIdx]);
					//renderer.rgbdata_[pixIdx] = fd::Color(255/(ty + 1),255/(tx + 1),0);
				}
			}
			double score = measureDetail(tile);
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

void dive(fd::Renderer& renderer, fd::Canvas& canvas) {
	renderer.render(true);
	double detail = measureDetail(renderer.greydata_);
	std::cerr << "detail:" << detail << std::endl;
	if (detail > 0.4) {
		renderer.zoomAt(renderer.WIDTH_ / 2, renderer.HEIGHT_ / 2, 1.05, true);
		renderer.render();
		canvas.draw(renderer.rgbdata_);
	} else {
		std::pair<int32_t,int32_t> centerOfHighDetail = identifyCenterOfTileOfHighestDetail(5, 5, renderer, canvas);
//		std::cerr << centerOfHighDetail.first << ":" << centerOfHighDetail.second << std::endl;
		renderer.zoomAt(centerOfHighDetail.first, centerOfHighDetail.second, 1.05, true);
		renderer.render();
		canvas.draw(renderer.rgbdata_);
	}
}

int run(size_t width, size_t height, size_t maxIterations) {
	fd::Renderer renderer(width, height, maxIterations);
	renderer.pan((rand() % 10) - 20, (rand() % 10) - 20);
	fd::Canvas canvas(width, height, false);
	while(do_run) {
		auto start = std::chrono::system_clock::now();

		dive(renderer, canvas);

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - start);

		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::cerr << 1000.0 / duration.count() << std::endl;
	}

	SDL_Quit();
	return 0;
}

void sigint_handler(int sig) {
	do_run = false;
}

int main() {
	srand (time(NULL));
	signal(SIGINT, sigint_handler);
	return run(WIDTH, HEIGHT, MAX_ITERATIONS);
}
