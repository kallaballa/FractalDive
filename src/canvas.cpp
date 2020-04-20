#include "canvas.hpp"

#include <iostream>
#include "color.hpp"

namespace fractaldive {
Canvas::Canvas(size_t width, size_t height, bool offscreen) :
  width_(width), height_(height), screen_(NULL), offscreen_(offscreen) {
  if (width > 0 && height > 0) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
      printf("Can't init SDL:  %s\n", SDL_GetError());
      exit(1);
    }
    atexit(SDL_Quit);

    if(!offscreen)
      screen_ = SDL_SetVideoMode(width, height, BYTES_PER_PIXEL * 8,0);
    else
      screen_ = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, BYTES_PER_PIXEL * 8, 0, 0, 0, 0);

    if (screen_ == NULL) {
      printf("Can't set video mode: %s\n", SDL_GetError());
      exit(1);
    }
  }
}

void Canvas::flip() {
  if(!offscreen_)
    SDL_Flip(screen_);
}

void Canvas::putpixel(const size_t& x, const size_t& y, const Color& c) {
	Uint8 *p = (Uint8 *) screen_->pixels + y * screen_->pitch + x * BYTES_PER_PIXEL;
	if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = c.r_;
			p[1] = c.g_;
			p[2] = c.b_;
		} else {
			p[0] = c.b_;
			p[1] = c.g_;
			p[2] = c.r_;
		}
}
void Canvas::draw(const std::vector<Color>& rgbdata) {
	for(size_t y = 0; y < height_; ++y) {
		for(size_t x = 0; x < width_; ++x) {
			putpixel(x, y, rgbdata[y * width_ + x]);
		}
	}
	flip();
}

}
