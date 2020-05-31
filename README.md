Note: While i think this code might by very interesting to a lot of people, this is strictly an educational fun-project. It also is very much still a work-in-progress and in some respects not optimal.

This is an attempt at implementing a highly optimized realtime mandelbrot fractal zoom in a highly platform independent fashion using modern C++. I started this project because I have a tattoo of the formular for the mandebrot set on my left arm and people kept asking what it is - so instead of describing it in my own words, every time, i decided to write this demo. At the prospect of optimizing javascript code (and fighting its nature) i decided to write in C++ and bow to the power of llvm by using emscripten :). And hey, If I'm writing it in C++ why not have it run on a bunch of platforms? :D

My most important guideline is a quick and nice out-of-box experience and therefore I implemented (a rather naive) automatic scaling of the zoom depth. Depending on platform capabilities a number of optional features is available, the most important being: threading, auto vectorization, fixed/floating point.

List of tested platforms are:
- Many kinds of browsers on mobile and desktop. Example: http://www.metastage.at/dive-mt/dive.html
- x86 Linux and MacOSX
- armv7 Linux
- Amiga 500 / 1200 (https://gfycat.com/simplebonyfairyfly) / 4000 (https://gfycat.com/adolescentkindamericanredsquirrel)

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

## JavaScript/WebAsm
To build for Javascript to need em++ > 1.39.1. The following builds might need specific browsers or even special browser configurations. In src/index.html you can an example of how to select the right javascript build to load by using feature checking.
```
make clean && JAVASCRIPT=1 make -j2 hardcore
```
### Multi-Threaded 
```
make clean && JAVASCRIPT_MT=1 make -j2 hardcode
```
### Using simd instructions
```
make clean && AUTOVECTOR=1 JAVASCRIPT=1 make -j2 hardcore
```
### Using multi-threading and simd instructions
```
make clean && AUTOVECTOR=1 JAVASCRIPT_MT=1 make -j2 hardcore
```
## Linux/MacOSX (x86 and armv7)
```
make clean && AUTOVECTOR=1 make -j2 hardcore
```
### Without multi-threading
```
make clean && NOTHREADS=1 AUTOVECTOR=1 make -j2 hardcode
```
## Amiga/m68k

For m68k you need amiga-gcc (https://github.com/kallaballa/amiga-gcc/releases/tag/latest-20200516174914).

### Build for 68000
```
make clean; AMIGA=68020 make CXX=m68k-amigaos-g++ LD=m68k-amigaos-ld hardcore
```

### Build for 68020
```
make clean; AMIGA=68020 make CXX=m68k-amigaos-g++ LD=m68k-amigaos-ld hardcore
```
