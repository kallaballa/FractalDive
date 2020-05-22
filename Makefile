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

ifndef JAVASCRIPT
ifndef AMIGA
CXXFLAGS += -march=native
endif
endif



ifdef AMIGA
FIXEDPOINT=1
NOTHREADS=1
CXXFLAGS += -mhard-float -mcrt=nix13 -D_AMIGA -Wa,-march=${AMIGA} -Wa,-mcpu=${AMIGA} -march=${AMIGA} -mtune=${AMIGA} -mcpu=${AMIGA} -fbbb=+
LDFLAGS+= -mcrt=nix13
endif

ifdef NOTHREADS
CXXFLAGS += -D_NO_THREADS
endif

ifdef JAVASCRIPT_MT
JAVASCRIPT=1
CXXFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD
LDFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD
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
EMFLAGS = -DNDEBUG -D_JAVASCRIPT -flto
# unsafe options
EMFLAGS += -ffp-contract=fast -freciprocal-math -fno-signed-zeros --closure 1 --llvm-opts "['-menable-no-infs', '-menable-no-nans', '-menable-unsafe-fp-math']" 
# emscripteb options
EMFLAGS +=  -s INITIAL_MEMORY=33554432 -s ASYNCIFY

ifdef AUTOVECTOR
EMFLAGS += -msimd128
endif

CXXFLAGS += $(EMFLAGS) 
LDFLAGS += $(EMFLAGS)
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
release: CXXFLAGS += -g0 -O3 -c
release: dirs

shrink: CXXFLAGS += -Os -w
shrink: LDFLAGS += -s
shrink: dirs

info: CXXFLAGS += -g3 -O0
info: LDFLAGS += -Wl,--export-dynamic -rdynamic
info: dirs

debug: CXXFLAGS += -g3 -O0 -rdynamic
debug: LDFLAGS += -Wl,--export-dynamic -rdynamic
debug: dirs

profile: CXXFLAGS += -g3 -O3 
profile: LDFLAGS += -Wl,--export-dynamic
ifdef JAVASCRIPT
profile: LDFLAGS += --profiling
profile: CXXFLAGS += --profiling
endif
ifndef AMIGA
profile: CXXFLAGS += -rdynamic
endif
profile: dirs

hardcore: CXXFLAGS += -g0 -Ofast
#ifeq ($(UNAME_S), Darwin)
hardcore: LDFLAGS += -s
#endif
hardcore: dirs

asan: CXXFLAGS += -g3 -O0 -rdynamic -fno-omit-frame-pointer -fsanitize=address
asan: LDFLAGS += -Wl,--export-dynamic -fsanitize=address
asan: LIBS+= -lbfd -ldw
asan: dirs

clean: dirs

export LDFLAGS
export CXXFLAGS
export LIBS

dirs:
	${MAKE} -C src/ ${MAKEFLAGS} CXX=${CXX} ${MAKECMDGOALS}

debian-release:
	${MAKE} -C src/ -${MAKEFLAGS} CXX=${CXX} release

debian-clean:
	${MAKE} -C src/ -${MAKEFLAGS} CXX=${CXX} clean

install: ${TARGET}
	true
	
distclean:
	true
