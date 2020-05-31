#ifndef CANVAS_H_
#define CANVAS_H_

#include <cwchar>
#include <SDL/SDL.h>

#include "types.hpp"

namespace fractaldive {
class Canvas {
private:
  fd_dim_t width_;
  fd_dim_t height_;
	class SDL_Surface *screen_;
  bool offscreen_;
  const int BYTES_PER_PIXEL = FD_IMAGE_DEPTH_IN_BYTES;
public:
  Canvas(const fd_dim_t& width, const fd_dim_t& height, const bool& offscreen = false);
  virtual ~Canvas() {};
  void flip();
#ifndef _NO_SHADOW
	void draw(const image_t& image);
#else
	void draw(const shadow_image_t& image);
#endif
};
}
#endif /* CANVAS_H_ */
