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

	//we have no real-world example where float precision is exceeded hence the default
	typedef float fd_float_t;

#ifdef _FIXEDPOINT
	#ifdef _AMIGA
	//on 000, 020 and 030 the multiplication of a 8bit+8bit in square translates to "muls.w d0,d0" and the precision seems to be enough for what we can achieve there
	typedef mn::MFixedPoint::FpF16<8> fd_mandelfloat_t;
	#else
	//Enough precision on all real-world examples we tested.
	typedef mn::MFixedPoint::FpF32<16> fd_mandelfloat_t;
	#endif
#else
	typedef fd_float_t fd_mandelfloat_t;
#endif

#ifndef _NO_SHADOW
	//usually we use a (cheaply generated) greyscale "shadow" of the color24_t image for the area of interest search.
	typedef uint8_t fd_shadow_pix_t;
	typedef fd_shadow_pix_t* shadow_image_t;
	typedef uint32_t fd_image_pix_t;
	constexpr int FD_IMAGE_DEPTH_IN_BYTES = 4;
	constexpr int FD_SHADOW_DEPTH_IN_BYTES = 1;
	typedef fd_mandelfloat_t fd_iter_count_t;
#else
	//_NO_SHADOW basically that Renderer::imgdata_ and Renderer::shadowdata_ are the same (uint16_t) pointer and that a very simple palette algorithm is used
	//8bit samples don't perform better on m68k 020/030 with a P96 (Zorro III) and on powerful platforms.
	typedef uint16_t fd_shadow_pix_t;
	typedef fd_shadow_pix_t* shadow_image_t;
	typedef uint16_t fd_image_pix_t;
	constexpr int FD_IMAGE_DEPTH_IN_BYTES = 2;
	constexpr int FD_SHADOW_DEPTH_IN_BYTES = 2;
	typedef fd_mandelfloat_t fd_iter_count_t;
#endif

	typedef fd_image_pix_t* image_t;
}

#endif /* SRC_TYPES_HPP_ */
