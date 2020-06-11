/*
 * util.hpp
 *
 *  Created on: Jun 11, 2020
 *      Author: elchaschab
 */

#ifndef SRC_UTIL_HPP_
#define SRC_UTIL_HPP_

#include <cstdint>
#include <SDL/SDL.h>
#include "types.hpp"

#ifdef _JAVASCRIPT
#include <emscripten.h>
#endif

namespace fractaldive {
inline fd_highres_tick_t get_milliseconds() {
	return SDL_GetTicks();
}

#ifndef _AMIGA
inline fd_highres_tick_t get_microseconds() {
	return std::chrono::duration_cast<std::chrono::microseconds>(highres_clock::now().time_since_epoch()).count();
}
#else
inline uint64_t get_microseconds() {
	assert(false);
	return 0;
}
#endif

inline fd_highres_tick_t get_highres_tick() {
#ifndef _AMIGA
	return get_microseconds();
#else
	return get_millisecond();
#endif
}

inline void sleep_millis(uint32_t millis) {
#ifdef _JAVASCRIPT
	emscripten_sleep(millis);
#else
	SDL_Delay(millis);
#endif
}
}
#endif /* SRC_UTIL_HPP_ */
