#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <cstdint>
#include <cstddef>

#include "color.hpp"

namespace fractaldive {
	typedef int32_t coord_t;
	typedef size_t dim_t;
	typedef float float_t;
	typedef uint8_t ccomp_t;
	typedef ccomp_t* grey_image_t;
	typedef Color<ccomp_t> color24_t;
	typedef color24_t* rgb_image_t;
	typedef Palette<ccomp_t> palette24_t;
}

#endif /* SRC_TYPES_HPP_ */
