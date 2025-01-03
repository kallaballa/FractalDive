CXX      := g++
CXXFLAGS := -std=c++0x -pedantic -Wall -fno-rtti -fno-exceptions
LDFLAGS  := -L/opt/local/lib 
LIBS     := -lm
.PHONY: all release debian-release info debug clean debian-clean distclean asan shrink
ESTDIR := /
PREFIX := /usr/local
MACHINE := $(shell uname -m)
UNAME_S := $(shell uname -s)
LIBDIR := lib

UNAME_P := $(shell uname -p)

ifeq ($(UNAME_P),x86_64)
LIBDIR = lib64
endif

ifdef BENCHMARK_ONLY
CXXFLAGS += -D_BENCHMARK_ONLY
endif

ifndef JAVASCRIPT
ifndef AMIGA
#CXXFLAGS += -march=native
endif
endif



ifdef AMIGA
FIXEDPOINT=1
NOTHREADS=1
NOSHADOW=1
CXXFLAGS += -mhard-float -mcrt=nix13 -D_AMIGA -Wa,-march=${AMIGA} -Wa,-mcpu=${AMIGA} -march=${AMIGA} -mtune=${AMIGA} -mcpu=${AMIGA} -fbbb=+
LDFLAGS+= -mcrt=nix13
endif

ifndef TIMETRACK
CXXFLAGS += -D_NO_TIMETRACK
endif

ifdef  
CXXFLAGS += -D_NO_SHADOW
endif

ifdef NOSHADOW 
CXXFLAGS += -D_NO_SHADOW
endif

ifdef NOTHREADS
CXXFLAGS += -D_NO_THREADS
endif

ifdef LOWRES
CXXFLAGS += -D_LOW_RES
else
  ifdef HIGHRES
CXXFLAGS += -D_HIGH_RES
  else
    ifdef ULTRARES
CXXFLAGS += -D_ULTRA_RES
    endif
  endif
endif

ifdef SLOWZOOM
CXXFLAGS += -D_SLOW_ZOOM
else
  ifdef FASTZOOM
CXXFLAGS += -D_FAST_ZOOM
  else
    ifdef FASTERZOOM
CXXFLAGS += -D_FASTER_ZOOM
    endif
  endif
endif



ifdef JAVASCRIPT_MT
JAVASCRIPT=1
CXXFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -pthread
LDFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD -pthread -s PTHREAD_POOL_SIZE=navigator.hardwareConcurrency
endif

ifeq ($(UNAME_S), Darwin)
 LDFLAGS += -L/opt/X11/lib/
else
ifndef JAVASCRIPT
# CXXFLAGS += -march=native
endif
endif

ifdef JAVASCRIPT
CXX	:= em++
# defines
EMCXXFLAGS = -D_JAVASCRIPT -flto
# emscripteb options
EMCXXFLAGS +=  -s DISABLE_EXCEPTION_CATCHING=1
ifndef JAVASCRIPT_MT
EMCXXFLAGS += -D_NO_THREADS
endif
EMLDFLAGS +=  -s INITIAL_MEMORY=2147483648 -s ASYNCIFY -s TOTAL_STACK=52428800 -s WASM_BIGINT -s MALLOC=emmalloc

ifdef AUTOVECTOR
EMCXXFLAGS += -msimd128
endif

CXXFLAGS += $(EMCXXFLAGS)
LDFLAGS += $(EMLDFLAGS)
endif

ifdef AUTOVECTOR
CXXFLAGS += -D_AUTOVECTOR
endif


ifdef FIXEDPOINT
ifdef AMIGA
CXXFLAGS +=-msoft-float -D_FIXEDPOINT
else
CXXFLAGS += -D_FIXEDPOINT
endif
endif

ifdef X86
CXXFLAGS += -m32
LDFLAGS += -L/usr/lib -m32 -static-libgcc -m32 -Wl,-Bstatic
endif

ifdef STATIC
LDFLAGS += -static-libgcc -Wl,-Bstatic
endif

all: release

ifneq ($(UNAME_S), Darwin)
release: LDFLAGS += -s
endif
ifdef JAVASCRIPT
release: LDFLAGS += -s STACK_OVERFLOW_CHECK=0 -s ASSERTIONS=0 -s SAFE_HEAP=0
endif
release: CXXFLAGS += -g0 -O3 -c
release: dirs

shrink: CXXFLAGS += -Os -w
shrink: LDFLAGS += -s
shrink: dirs

info: CXXFLAGS += -g3 -O0
info: LDFLAGS += -Wl,--export-dynamic -rdynamic
info: dirs

ifndef JAVASCRIPT
debug: CXXFLAGS += -rdynamic
debug: LDFLAGS += -rdynamic
endif
debug: CXXFLAGS += -g3 -O0
debug: LDFLAGS += -Wl,--export-dynamic
debug: dirs

profile: CXXFLAGS += -g3 -O3 
profile: LDFLAGS += -Wl,--export-dynamic
ifdef JAVASCRIPT
profile: LDFLAGS += --profiling
profile: CXXFLAGS += --profiling
endif
ifndef AMIGA
ifndef JAVASCRIPT
profile: CXXFLAGS += -rdynamic
endif
endif
profile: dirs

ifdef JAVASCRIPT
hardcore: CXXFLAGS += -DNDEBUG -g0 -O3 -ffp-contract=fast -freciprocal-math -fno-signed-zeros
hardcore: LDFLAGS += -s STACK_OVERFLOW_CHECK=0 -s ASSERTIONS=1 -s SAFE_HEAP=0 -menable-unsafe-fp-math --closure 1
else
hardcore: CXXFLAGS += -DNDEBUG -g0 -Ofast
endif
#ifeq ($(UNAME_S), Darwin)
hardcore: LDFLAGS += -s
#endif
hardcore: dirs

ifdef JAVASCRIPT
#asan: CXXFLAGS += -fsanitize=undefined
asan: LDFLAGS += -s STACK_OVERFLOW_CHECK=2 -s ASSERTIONS=2 #-fsanitize=undefined
else
asan: CXXFLAGS += -rdynamic -fsanitize=address
asan: LDFLAGS += -rdynamic -fsanitize=address
endif
asan: CXXFLAGS += -g3 -O0 -fno-omit-frame-pointer
asan: LDFLAGS += -Wl,--export-dynamic 
ifndef JAVASCRIPT
asan: LIBS+= -lbfd -ldw
endif
asan: dirs

clean: dirs

export LDFLAGS
export CXXFLAGS
export LIBS

dirs:
	${MAKE} -C src/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}
#	${MAKE} -C exp/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}

debian-release:
	${MAKE} -C src/ -${MAKEFLAGS} CXX=${CXX} release
#	${MAKE} -C exp/ -${MAKEFLAGS} CXX=${CXX} release
	
debian-clean:
	${MAKE} -C src/ -${MAKEFLAGS} CXX=${CXX} clean
#	${MAKE} -C exp/ -${MAKEFLAGS} CXX=${CXX} clean
	
install: ${TARGET}
	true
	
distclean:
	true
