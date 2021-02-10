#ifndef SRC_IMAGEDETAIL_HPP_
#define SRC_IMAGEDETAIL_HPP_

#include "types.hpp"
//#include <spatium/Image.h>
//#include <spatium/ImageIO.h>
//#include <spatium/imgproc/GlobalThreshold.h>
//#include <spatium/imgproc/Grayscale.h>
//#include <spatium/imgproc/Blur.h>
//#include <spatium/imgproc/Sobel.h>

//using namespace spatium;

namespace fractaldive {
/*
fd_float_t entropy(const image_t& image, const size_t& size) {
	std::map<uint8_t, int32_t> frequencies;

	for (size_t i = 0; i < size; ++i)
		++frequencies[image[i]];

	fd_float_t infocontent = 0;
	for (const auto& p : frequencies) {
		fd_float_t freq = static_cast<fd_float_t>(p.second) / size;
		infocontent -= freq * log2(freq);
	}

	return infocontent;
}

fd_float_t numberOfZeroes(const image_t& image, const size_t& size) {
	assert(size > 0);
	fd_float_t zeroes = 0;
	for (size_t i = 0; i < size; i++) {
		if (image[i] == 0)
			++zeroes;
	}
	return zeroes / size;
}

fd_float_t numberOfColors(const image_t& image, const size_t& size) {
	assert(size > 0);
	std::set<fd_image_pix_t> clrs;
	for (size_t i = 0; i < size; i++) {
		clrs.insert(image[i]);
	}
	return (fd_float_t(clrs.size()) / size);
}
*/

//fd_float_t blurCount(const image_t& image, const fd_dim_t& width, const fd_dim_t& height) {
//	Image<unsigned char, 3> imageRgb(width, height);
//	for(fd_dim_t y = 0; y < height; ++y) {
//		for(fd_dim_t x = 0; x < width; ++x) {
//			imageRgb.pixel(x,y)[0] = image[y * width + x];
//		}
//	}
//	Image<unsigned char, 1> imageGray(imageRgb.width(), imageRgb.height());
//	imgproc::Grayscale<> grayscale;
//	grayscale.apply(imageRgb, imageGray);
//
//	Image<unsigned char, 1> imageSobel(imageRgb.width(), imageRgb.height());
//	imgproc::Sobel sobel;
//	sobel.apply(imageGray, imageSobel);
//
//	imgproc::GlobalThreshold<unsigned char> filter(20);
//	Image<unsigned char, 1> imageThreshold(imageRgb.width(), imageRgb.height());
//	filter.apply(imageSobel, imageThreshold);
//
//	Image<unsigned char, 1> imageBlur(imageRgb.width(), imageRgb.height());
//	imgproc::Blur blur;
//	blur.apply(imageThreshold, imageBlur);
//
//	fd_float_t numChanges = 0;
//	unsigned char last = 0;
//
//	for(fd_dim_t y = 0; y < height; ++y) {
//		for(fd_dim_t x = 0; x < width; ++x) {
//			const unsigned char& p = imageThreshold.pixel(x,y)[0];
//			if (last != p) {
//				++numChanges;
//			}
//			last = p;
//		}
//	}
//
//	return (fd_float_t(numChanges) / fd_float_t(width * height));
//}

inline fd_float_t numberOfChanges(const image_t& image, const size_t& size) {
	fd_float_t numChanges = 0;
	fd_image_pix_t last = 0;
	for (size_t i = 0; i < size; i++) {
		const fd_image_pix_t& p = image[i];
		if (last != p) {
			++numChanges;
		}
		last = p;
	}

	return (fd_float_t(numChanges) / fd_float_t(size));
}

inline fd_float_t measureImageDetail(const image_t& image, const size_t& size) {
	return numberOfChanges(image, size);
}
}

#endif /* SRC_IMAGEDETAIL_HPP_ */
