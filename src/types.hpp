#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <cstdint>
#include <cstddef>
#include <limits>

#ifdef _FIXEDPOINT
#include "MFixedPoint/FpF.hpp"
#endif

#include "color.hpp"

namespace fractaldive {
	typedef int32_t fd_coord_t;
	typedef size_t fd_dim_t;
	typedef double fd_float_t;
#ifdef _FIXEDPOINT
	typedef mn::MFixedPoint::FpF32<16> fd_mandelfloat_t;
#else
	typedef double fd_mandelfloat_t;
#endif
	typedef uint8_t fd_ccomp_t;
	typedef fd_ccomp_t* grey_image_t;
	typedef Color<fd_ccomp_t> color24_t;
	typedef color24_t* rgb_image_t;
	typedef Palette<fd_ccomp_t> palette24_t;
}

#endif /* SRC_TYPES_HPP_ */
