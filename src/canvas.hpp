#ifndef CANVAS_H_
#define CANVAS_H_

#include <algorithm>
#include <string>
#include <SDL/SDL.h>
#include <vector>

#include "color.hpp"

using std::string;

namespace fractaldive {
class Canvas {
private:
  size_t width_;
  size_t height_;
	class SDL_Surface *screen_;
  bool offscreen_;
  const uint8_t BYTES_PER_PIXEL = 3;
public:
  Canvas(size_t width, size_t height, bool offscreen = false);
  virtual ~Canvas() {};
  void putpixel(const size_t& x, const size_t& y, const Color& c);
  void flip();
  void draw(const std::vector<Color>& rgbdata);
};
}
#endif /* CANVAS_H_ */
