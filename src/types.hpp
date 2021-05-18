#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <cstdint>
#include <cstddef>

#ifdef _FIXEDPOINT
#include "MFixedPoint/FpF.hpp"
#endif

#ifndef _AMIGA
#include <chrono>
#endif

#include "color.hpp"

namespace fractaldive {
#ifndef _AMIGA
  typedef double fd_float_t;
  typedef int64_t fd_coord_t;
  typedef uint64_t fd_dim_t;
  typedef uint32_t fd_image_pix_t;
  constexpr int FD_IMAGE_DEPTH_IN_BYTES = 4;
  typedef uint32_t fd_iter_count_t;
  typedef fd_image_pix_t* image_t;
  typedef std::chrono::high_resolution_clock highres_clock;
  typedef std::chrono::microseconds::rep fd_highres_tick_t;
#else
  typedef float fd_float_t;
  typedef int16_t fd_coord_t;
  typedef uint16_t fd_dim_t;
  typedef uint8_t fd_image_pix_t;
  constexpr int FD_IMAGE_DEPTH_IN_BYTES = 1;
  typedef uint16_t fd_iter_count_t;
  typedef fd_image_pix_t* image_t;
  typedef uint32_t fd_highres_tick_t;
#endif

#ifdef _FIXEDPOINT
	#ifdef _AMIGA
	//on 000, 020 and 030 the multiplication of a 8bit+8bit in square translates to "muls.w d0,d0" and the precision seems to be enough for what we can achieve there
	typedef mn::MFixedPoint::FpF16<8> fd_mandelfloat_t;
	constexpr char FD_PRECISION[] = "8bit/8bit";
	#else
	//Enough precision on all real-world examples we tested.
	typedef mn::MFixedPoint::FpF32<16> fd_mandelfloat_t;
	constexpr char FD_PRECISION[] = "16bit/16bit";
	#endif
#else
	typedef float fd_mandelfloat_t;
	constexpr char FD_PRECISION[] = "double";
#endif

}

#endif /* SRC_TYPES_HPP_ */
