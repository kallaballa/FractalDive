#include <cassert>

#include "canvas.hpp"
#include "printer.hpp"
#include "color.hpp"

namespace fractaldive {
Canvas::Canvas(const fd_dim_t& width, const fd_dim_t& height, const bool& offscreen) :
  width_(width), height_(height), screen_(NULL), offscreen_(offscreen) {
	assert(BYTES_PER_PIXEL > 1); //SDL doesn't support 8bit images. It interpretes 8bit as paletted image

	if (width > 0 && height > 0) {
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
    	printErr("Can't init SDL: ", SDL_GetError());
      exit(1);
    }
    atexit(SDL_Quit);

    if(!offscreen) {
#ifndef _AMIGA
    	screen_ = SDL_SetVideoMode(width, height, BYTES_PER_PIXEL * 8, SDL_SWSURFACE);
#else
    	screen_ = SDL_SetVideoMode(width, height, BYTES_PER_PIXEL * 8, SDL_SWSURFACE | SDL_FULLSCREEN);
#endif
    }
    else
      screen_ = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, BYTES_PER_PIXEL * 8, 0, 0, 0, 0);

    if (screen_ == NULL) {
    	printErr("Can't set video mode: ", SDL_GetError());
      exit(1);
    }
  }
}

void Canvas::flip() {
	if(!offscreen_) {
    SDL_Flip(screen_);
  }
}
#ifndef _NO_SHADOW
	void Canvas::draw(image_t const& image) {
#else
	void Canvas::draw(shadow_image_t const& image) {
#endif
	if (SDL_MUSTLOCK(screen_))
		SDL_LockSurface(screen_);

	/* in cases where image_t actually is a "Color<T>" pointer this is undefined behaviour,
	 * but we seem to get away with it because there is not rtti.
	 * let's use it till it breaks and rework the "constexpr Color" approach. :)
	 */
	//blur(image);
	memcpy(static_cast<void*>(screen_->pixels), static_cast<void*>(image), width_ * height_ * BYTES_PER_PIXEL);
	flip();

	if (SDL_MUSTLOCK(screen_))
		SDL_UnlockSurface(screen_);
}

}

