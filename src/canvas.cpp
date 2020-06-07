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

void Canvas::blur(image_t& image) //This manipulates with SDL_Surface and gives it box blur effect
{
	fd_coord_t blur_extent = 1;
	for (int y = 0; y < width_; y++)
	{
		for (int x = 0; x < height_; x++)
		{
			Uint32 color = (((Uint32*)image)[y * width_ + x]);
			//SDL_GetRGBA() is a method for getting color
			//components from a 32 bit color
			Uint8 r = 0, g = 0, b = 0, a = 0;
			SDL_GetRGBA(color, screen_->format, &r, &g, &b, &a);

			Uint32 rb = 0, gb = 0, bb = 0, ab = 0;

			//Within the two for-loops below, colors of adjacent pixels are added up

			for (int yo = -blur_extent; yo <= blur_extent; yo++)
			{
				for (int xo = -blur_extent; xo <= blur_extent; xo++) 				{ 					if (y + yo >= 0 && x + xo >= 0
						&& y + yo < height_ && x + xo < width_
						)
					{
						Uint32  colOth = (((Uint32*)image)[(y + yo) * width_ + (x + xo)]);

						Uint8 ro = 0, go = 0, bo = 0, ao = 0;
						SDL_GetRGBA(colOth, screen_->format, &ro, &go, &bo, &ao);

						rb += ro;
						gb += go;
						bb += bo;
						ab += ao;
					}
				}
			}

			//The sum is then, divided by the total number of
			//pixels present in a block of blur radius

			//For blur_extent 1, it will be 9
			//For blur_extent 2, it will be 25
			//and so on...

			//In this way, we are getting the average of
			//all the pixels in a block of blur radius

			//(((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)) calculates
			//the total number of pixels present in a block of blur radius

			r = (Uint8)(rb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));
			g = (Uint8)(gb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));
			b = (Uint8)(bb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));
			a = (Uint8)(ab / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));

			//Bit shifting color bits to form a 32 bit proper colour
			color = (r) | (g << 8) | (b << 16) | (a << 24);
			((Uint32*)image)[y * width_ + x] = color;
		}
	}
}
void Canvas::flip() {
	if(!offscreen_) {
    SDL_Flip(screen_);
  }
}
#ifndef _NO_SHADOW
	void Canvas::draw(image_t& image) {
#else
	void Canvas::draw(shadow_image_t& image) {
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

