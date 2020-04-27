#ifndef CANVAS_H_
#define CANVAS_H_

#include <algorithm>
#include <string>
#include <SDL/SDL.h>
#include <vector>

#include "types.hpp"

using std::string;

namespace fractaldive {
class Canvas {
private:
  fd_dim_t width_;
  fd_dim_t height_;
	class SDL_Surface *screen_;
  bool offscreen_;
#ifdef _JAVASCRIPT
  const uint8_t BYTES_PER_PIXEL = 4;
#else
  const uint8_t BYTES_PER_PIXEL = 3;
#endif

public:
  Canvas(const fd_dim_t& width, const fd_dim_t& height, bool offscreen = false);
  virtual ~Canvas() {};
  void putpixel(const fd_coord_t& x, const fd_coord_t& y, const color24_t& c);
  void flip();
  void draw(const rgb_image_t& rgbdata);
};
}
#endif /* CANVAS_H_ */
