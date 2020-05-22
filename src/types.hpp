#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <cstdint>
#include <cstddef>
#include <limits>
#define _FIXEDPOINT 1

#ifdef _FIXEDPOINT
#include "MFixedPoint/FpF.hpp"
#endif

#include "color.hpp"

namespace fractaldive {
	typedef int16_t fd_coord_t;
	typedef uint16_t fd_dim_t;
	typedef float fd_float_t;
#ifdef _FIXEDPOINT
	#ifdef _AMIGA
	typedef mn::MFixedPoint::FpF16<8> fd_mandelfloat_t;
	#else
	typedef mn::MFixedPoint::FpF32<16> fd_mandelfloat_t;
	#endif
#else
	typedef float fd_mandelfloat_t;
#endif
	typedef uint8_t fd_ccomp_t;
	typedef fd_ccomp_t* grey_image_t;
	typedef Color<fd_ccomp_t> color24_t;
	typedef color24_t* rgb_image_t;
}

#endif /* SRC_TYPES_HPP_ */
