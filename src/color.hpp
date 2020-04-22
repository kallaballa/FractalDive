#ifndef SRC_COLOR_HPP_
#define SRC_COLOR_HPP_

#include <cstdint>
#include <cstddef>

namespace fractaldive {

template<typename T> struct Color {
	constexpr Color() : r_(0), g_(0), b_(0) {
	}

	constexpr Color(const T& r, const T& g, const T& b) : r_(r), g_(g), b_(b) {
	}
	constexpr Color(const uint32_t& rgb) : r_(rgb >> 16 & 0xFF), g_(rgb >> 8 & 0xFF), b_(rgb & 0xFF) {
	}

	T r_;
	T g_;
	T b_;
};

template <typename T> struct Palette {
	constexpr static size_t SIZE_ = 519;
	const static Color<T> ITEMS_[];
};

} /* namespace fractaldive */

#endif /* SRC_COLOR_HPP_ */
