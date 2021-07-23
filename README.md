![FractalDive on an Amiga 4000](https://viel-zu.org/amiga4000.gif "FractalDive on an Amiga 4000")

***Note***: While i think this code might by very interesting to a lot of people, this is strictly an educational fun-project. It also is very much still a work-in-progress and in some respects not optimal.

This is an attempt at implementing a highly optimized realtime mandelbrot fractal zoom in a highly platform independent fashion using modern C++. I started this project because I have a tattoo of the formular for the mandebrot set on my left arm and people kept asking what it is - so instead of describing it in my own words, every time, i decided to write this demo. At the prospect of optimizing javascript code (and fighting its nature) i decided to write it in C++ and bow to the power of llvm by using emscripten :). And hey, If I'm writing it in C++ why not have it run on a bunch of platforms? :D

My most important guideline is a quick and nice out-of-box experience and therefore I implemented automatic scaling of the iteration depth and the program automatically selects areas of interest. Depending on platform capabilities a number of optional features is available, the most important being: threading, auto vectorization, fixed/floating point.

List of tested platforms are:
- Many kinds of browsers on mobile and desktop. Example: http://viel-zu.org/
- x86 Linux and MacOSX
- armv7 Linux
- Amiga 1200 / 4000. Example: https://vimeo.com/578666410

The list of supported platforms probably is way longer.

# Building

The only direct dependency is SDL1. You don't have to install SDL1 when compiling for javascript or m68k (amiga).

## Build targets

There are many build targets. The default is "release" but on some platforms you can squeeze out a little performance by using "hardcore".

* "debug": optimize the build for interactive debugging.
* "info": retain minimal debug information but try to stay highly optimized
* "profile": optimized the build for profiling
* "hardcore": use non-standard, inaccurate and sometimes unsafe compiler options to squeeze out more performance.
* "asan": compile and optimize for AddressSanitizer (https://en.wikipedia.org/wiki/AddressSanitizer)
* "shrink": optimize for size

## JavaScript/WASM
To build for Javascript you need em++ > 2.0.24. The following builds might need specific browsers or even special browser configurations. In src/index.html you can an example of how to select the right javascript build to load by using feature checking.
```bash
make clean && JAVASCRIPT=1 make -j2 hardcore
```
### Multi-Threaded 
```bash
make clean && JAVASCRIPT_MT=1 make -j2 hardcore
```
### Using simd instructions
```bash
make clean && AUTOVECTOR=1 JAVASCRIPT=1 make -j2 hardcore
```
### Using multi-threading and simd instructions
```bash
make clean && AUTOVECTOR=1 JAVASCRIPT_MT=1 make -j2 hardcore
```
## Linux/MacOSX (x86 and armv7)
```bash
make clean && AUTOVECTOR=1 make -j2 hardcore
```
### Without multi-threading
```bash
make clean && NOTHREADS=1 AUTOVECTOR=1 make -j2 hardcode
```
## Amiga/m68k

For m68k you need amiga-gcc (https://github.com/kallaballa/amiga-gcc/releases/tag/latest-20200516174914).

### Build for 68000
```bash
make clean; AMIGA=68000 make CXX=m68k-amigaos-g++ LD=m68k-amigaos-ld hardcore
```

### Build for 68020
```bash
make clean; AMIGA=68020 make CXX=m68k-amigaos-g++ LD=m68k-amigaos-ld hardcore
```
# Optimizations

## Core algorithm

The program naturally spends most of the time in the mandelbrot fraktal rendering algorithm so that's where i put most of the work.
I found that there are many apects to consider in order to get cross-platform and high performance code with C++ only. I will outline the most important considerations and optimizations using the following code snippets.

## Code

### Before
```C++
float x0 = (x + offsetx_ + panx_) / (zoom_ / 10);
float y0 = (y + offsety_ + pany_) / (zoom_ / 10);
std::complex<float> point(x0/width_, y0/height_);
std::complex<float> z(0, 0);
size_t iterations = 0;
while (abs (z) < 2 && iterations < maxIterations_) {
	z = z * z + point;
	++iterations;
}
```

### After
```C++
fd_iter_count_t iterations = 0;
fd_mandelfloat_t x0 = (x + offsetx_ + panx_) / (zoom_ / 10.0);
fd_mandelfloat_t y0 = (y + offsety_ + pany_) / (zoom_ / 10.0);

fd_mandelfloat_t zr = 0.0, zi = 0.0;
fd_mandelfloat_t zrsqr = 0;
fd_mandelfloat_t zisqr = 0;
fd_mandellongfloat_t pointr = x0 / width_; //0.0 - 1.0
fd_mandellongfloat_t pointi = y0 / height_; //0.0 - 1.0
fd_mandelfloat_t four = 4.0;

while (iterations < maxIterations_ && zrsqr + zisqr <= four) {
	zi = (zr + zr) * zi;
	zi += pointi;
	zr = (zrsqr - zisqr) + pointr;

	zrsqr = square(zr);
	zisqr = square(zi);
	++iterations;
}
```
## Considerations
### Mind the compiler backend
Compiler backends (the part of the compiler that generates machine code) may differ greatly. That is because while C++ is very thoroughly specified as a programming language, its exact behaviour as a result of machine specifics is often undefined. e.g. What is the complexity of std::pow on any given machine for decimals and floats? How many instructions does it take to multiply a uint32_t? How about register allocation?
Also, what kind of optimizations can or will be applied may be very different. Therefore an important goal of the optimizations efforts is to seek an optimized version of the algorithm that yields in high performance for all targets.

### Precision
For zooms much deeper, than what we can calculate in real-time, precision is a real issue. But knowing that we can't zoom that deep we can use very low precision arithmetic (floating and fixed-point). Also all kinds of approximations and other optimizations that affect precision should be considered as long as they don't impair "beauty".

### Types
The chosen datatypes for the algorithm heavily affect performance. e.g.: On a machine with only 8bit registers any operation on a 64-bit integer is very costly. Also certain platform specific automatic optimizations (e.g. vectorization and simd instructions) require very strategically chosen datatypes. At the moment all significant type definitions reside in "types.hpp" and are configured by passing compiler flags.
