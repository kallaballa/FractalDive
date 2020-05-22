#include "canvas.hpp"
#include "printer.hpp"
#include "color.hpp"

namespace fractaldive {
Canvas::Canvas(const fd_dim_t& width, const fd_dim_t& height, const bool& offscreen, const fd_dim_t& scale) :
  width_(width), height_(height), screen_(NULL), offscreen_(offscreen), scale_(scale) {
	Printer& printer = Printer::getInstance();
	if(scale < 1) {
  	printer.printErr("Canvas scale must be >= 1");
    exit(1);
	}

  if (width > 0 && height > 0) {
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
    	printer.printErr("Can't init SDL: ", SDL_GetError());
      exit(1);
    }
    atexit(SDL_Quit);

    if(!offscreen) {
#ifndef _AMIGA
    	screen_ = SDL_SetVideoMode(width * scale, height * scale, BYTES_PER_PIXEL * 8, SDL_SWSURFACE);
#else
    	screen_ = SDL_SetVideoMode(width * scale, height * scale, BYTES_PER_PIXEL * 8, SDL_SWSURFACE | SDL_FULLSCREEN);
#endif
    }
    else
      screen_ = SDL_CreateRGBSurface(SDL_SWSURFACE, width * 2, height * 2, BYTES_PER_PIXEL * 8, 0, 0, 0, 0);

    if (screen_ == NULL) {
    	printer.printErr("Can't set video mode: ", SDL_GetError());
      exit(1);
    }
  }
}

void Canvas::flip() {
	if(!offscreen_) {
    SDL_Flip(screen_);
  }
}

inline void Canvas::putpixel(const fd_coord_t& x, const fd_coord_t& y, const color24_t& c) {
	return;
	Uint8 *p = (Uint8 *) screen_->pixels + y * screen_->pitch + x * BYTES_PER_PIXEL;
#ifdef _JAVASCRIPT
	p[0] = c.r_;
	p[1] = c.g_;
	p[2] = c.b_;
	p[3] = 0;
#else
	if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		p[0] = c.r_;
		p[1] = c.g_;
		p[2] = c.b_;
	} else {
		p[0] = c.b_;
		p[1] = c.g_;
		p[2] = c.r_;
	}
#endif
}
void Canvas::draw(const rgb_image_t& rgbdata) {
	if (SDL_MUSTLOCK(screen_))
		SDL_LockSurface(screen_);

	memcpy(static_cast<void*>(screen_->pixels), static_cast<void*>(rgbdata), width_ * height_ * BYTES_PER_PIXEL);
	/*
	for(fd_dim_t y = 0; y < height_; ++y) {
		for(fd_dim_t x = 0; x < width_; ++x) {
			const color24_t& c = rgbdata[y * width_ + x];
			for(size_t xb = 0; xb < scale_; ++xb)
				for(size_t yb = 0; yb < scale_; ++yb) {
					putpixel((x * scale_) + xb, (y * scale_) + yb, c);
				}
		}

	}
*/
	flip();

	if (SDL_MUSTLOCK(screen_))
		SDL_UnlockSurface(screen_);
}

}
