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
#ifndef _AMIGA
	typedef int32_t fd_coord_t;
	typedef uint32_t fd_dim_t;
#else
	typedef int16_t fd_coord_t;
	typedef uint16_t fd_dim_t;
#endif

	typedef float fd_float_t;

#ifdef _FIXEDPOINT
	#ifdef _AMIGA
	typedef mn::MFixedPoint::FpF16<8> fd_mandelfloat_t;
	#else
	typedef mn::MFixedPoint::FpF32<16> fd_mandelfloat_t;
	#endif
#else
	typedef fd_float_t fd_mandelfloat_t;
#endif

#ifndef _NO_SHADOW
	typedef uint8_t fd_shadow_comp_t;
	typedef fd_shadow_comp_t* shadow_image_t;
	typedef Color<fd_shadow_comp_t> fd_image_comp_t;
	constexpr int FD_IMAGE_DEPTH_IN_BYTES = 4;
	constexpr int FD_SHADOW_DEPTH_IN_BYTES = 1;
	typedef fd_mandelfloat_t fd_iter_count_t;
#else
	typedef uint16_t fd_shadow_comp_t;
	typedef fd_shadow_comp_t* shadow_image_t;
	typedef uint16_t fd_image_comp_t;
	constexpr int FD_IMAGE_DEPTH_IN_BYTES = 2;
	constexpr int FD_SHADOW_DEPTH_IN_BYTES = 2;
	typedef fd_mandelfloat_t fd_iter_count_t;
#endif

	typedef fd_image_comp_t* image_t;
}

#endif /* SRC_TYPES_HPP_ */
