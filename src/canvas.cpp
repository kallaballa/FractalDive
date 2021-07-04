#include <cassert>

#include "canvas.hpp"
#include "printer.hpp"
#include "color.hpp"

namespace fractaldive {
Canvas::Canvas(const fd_dim_t& width, const fd_dim_t& height, const bool& offscreen) :
		width_(width), height_(height), screen_(NULL), offscreen_(offscreen) {

	if (width > 0 && height > 0) {
		if (SDL_Init(SDL_INIT_VIDEO) == -1) {
			printErr("Can't init SDL: ", SDL_GetError());
			exit(1);
		}
		atexit(SDL_Quit);

		if (!offscreen) {
#ifndef _AMIGA
			screen_ = SDL_SetVideoMode(width, height, BYTES_PER_PIXEL * 8, SDL_SWSURFACE);
#else
			screen_ = SDL_SetVideoMode(width, height, BYTES_PER_PIXEL * 8, SDL_SWSURFACE | SDL_FULLSCREEN);
#endif
		} else
			screen_ = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, BYTES_PER_PIXEL * 8, 0, 0, 0, 0);

		if (screen_ == NULL) {
			printErr("Can't set video mode: ", SDL_GetError());
			exit(1);
		}
	}
}

void Canvas::flip() {
	if (!offscreen_) {
		SDL_Flip(screen_);
	}
}

void Canvas::draw(image_t const& image) {
	if (SDL_MUSTLOCK(screen_))
		SDL_LockSurface(screen_);

	memcpy(static_cast<void*>(screen_->pixels), static_cast<void*>(image), width_ * height_ * sizeof(fd_image_pix_t));
	flip();

	if (SDL_MUSTLOCK(screen_))
		SDL_UnlockSurface(screen_);
}

}
