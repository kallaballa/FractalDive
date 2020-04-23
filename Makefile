CXX      := g++
CXXFLAGS := -fno-strict-aliasing -std=c++0x -pedantic -Wall
LDFLAGS  := -L/opt/local/lib 
LIBS     := -lm
.PHONY: all release debian-release info debug clean debian-clean distclean asan
ESTDIR := /
PREFIX := /usr/local
MACHINE := $(shell uname -m)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
 LDFLAGS += -L/opt/X11/lib/
else
 CXXFLAGS += -march=native
endif

ifeq ($(MACHINE), x86_64)
  LIBDIR = lib64
endif
ifeq ($(MACHINE), i686)
  LIBDIR = lib
endif

ifdef JAVASCRIPT_MT
JAVASCRIPT=1
CXXFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD
LDFLAGS += -D_JAVASCRIPT_MT -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD
endif

ifdef JAVASCRIPT
CXX	:= em++
# clang options
EMFLAGS = -O3 -g0 -flto -fno-rtti -fno-exceptions -ffp-contract=fast -freciprocal-math -fno-signed-zeros -DNDEBUG -D_JAVASCRIPT
# em++ options partially passed to llvm
EMFLAGS += --closure 1 --llvm-opts "['-menable-no-infs', '-menable-no-nans', '-menable-unsafe-fp-math']"
# em++ flags
EMFLAGS +=  -s INITIAL_MEMORY=33554432 -s ASYNCIFY

CXXFLAGS += $(EMFLAGS)
LDFLAGS += $(EMFLAGS)
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
release: CXXFLAGS += -g0 -O3
ifdef JAVASCRIPT
release: dirs
else
release: hardcore
endif

info: CXXFLAGS += -g3 -O0
info: LDFLAGS += -Wl,--export-dynamic -rdynamic
info: dirs

debug: CXXFLAGS += -g3 -O0 -rdynamic
debug: LDFLAGS += -Wl,--export-dynamic -rdynamic
debug: dirs

profile: CXXFLAGS += -g3 -O1
profile: LDFLAGS += -Wl,--export-dynamic -rdynamic
profile: dirs

hardcore: CXXFLAGS += -g0 -Ofast -DNDEBUG
ifeq ($(UNAME_S), Darwin)
hardcore: LDFLAGS += -s
endif
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
