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
  fd_dim_t scale_;
#ifdef _JAVASCRIPT
  const uint8_t BYTES_PER_PIXEL = 4;
#else
  const uint8_t BYTES_PER_PIXEL = 4;
#endif

public:
  Canvas(const fd_dim_t& width, const fd_dim_t& height, const bool& offscreen = false, const fd_dim_t& scale = 1);
  virtual ~Canvas() {};
  void putpixel(const fd_coord_t& x, const fd_coord_t& y, const color24_t& c);
  void flip();
  void draw(const rgb_image_t& rgbdata);
};
}
#endif /* CANVAS_H_ */
