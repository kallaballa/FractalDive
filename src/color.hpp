#ifndef SRC_COLOR_HPP_
#define SRC_COLOR_HPP_

#include <cstdint>
#include <cstddef>

namespace fractaldive {

struct Color {
	constexpr Color() : r_(0), g_(0), b_(0) {
	}

	constexpr Color(const uint8_t& r, const uint8_t& g, const uint8_t& b) : r_(r), g_(g), b_(b) {
	}
	constexpr Color(const uint32_t& rgb) : r_(rgb >> 16 & 0xFF), g_(rgb >> 8 & 0xFF), b_(rgb & 0xFF) {
	}

	uint8_t r_;
	uint8_t g_;
	uint8_t b_;
};

struct Palette {
	constexpr static size_t SIZE_ = 519;
	const static Color ITEMS_[];
};

} /* namespace fractaldive */

#endif /* SRC_COLOR_HPP_ */
