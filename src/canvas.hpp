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
  dim_t width_;
  dim_t height_;
	class SDL_Surface *screen_;
  bool offscreen_;
#ifdef _JAVASCRIPT
  const uint8_t BYTES_PER_PIXEL = 4;
#else
  const uint8_t BYTES_PER_PIXEL = 3;
#endif

public:
  Canvas(const dim_t& width, const dim_t& height, bool offscreen = false);
  virtual ~Canvas() {};
  void putpixel(const coord_t& x, const coord_t& y, const color24_t& c);
  void flip();
  void draw(const rgb_image_t& rgbdata);
};
}
#endif /* CANVAS_H_ */
