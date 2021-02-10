TARGET  := dive

ifdef JAVASCRIPT
TARGET := dive.js
endif

ifdef JAVASCRIPT_MT
TARGET := dive.js
endif

SRCS  := main.cpp renderer.cpp canvas.cpp threadpool.cpp printer.cpp config.cpp color.cpp camera.cpp

CXXFLAGS += -I../third/SpatiumLib-master/include/
ifndef JAVASCRIPT
ifndef JAVASCRIPT_MT
ifndef AMIGA
CXXFLAGS += `pkg-config --cflags sdl x11`
LIBS += `pkg-config --libs sdl x11`
else
LIBS += -lSDL -ldebug
endif
endif
endif
#precompiled headers
HEADERS := 

GCH     := ${HEADERS:.h=.gch}
OBJS    := ${SRCS:.cpp=.o} 
DEPS    := ${SRCS:.cpp=.dep} 

.PHONY: all release debug clean distclean 

all: release
release: ${TARGET}
debug: ${TARGET}
info: ${TARGET}
profile: ${TARGET}
hardcore: ${TARGET}
asan: ${TARGET}
shrink: ${TARGET}
assembler: ${TARGET}

${TARGET}: ${OBJS} 
	${CXX} ${LDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.cpp %.dep ${GCH}
	${CXX} ${CXXFLAGS} -o $@ -c $< 

${DEPS}: %.dep: %.cpp Makefile 
	${CXX} ${CXXFLAGS} -MM $< > $@ 

${GCH}: %.gch: ${HEADERS} 
	${CXX} ${CXXFLAGS} -o $@ -c ${@:.gch=.h}

install:
	mkdir -p ${DESTDIR}/${PREFIX}
	cp ${TARGET} ${DESTDIR}/${PREFIX}

uninstall:
	rm ${DESTDIR}/${PREFIX}/${TARGET}

clean:
	rm -f *~ ${DEPS} ${OBJS} ${GCH} ${TARGET} 

distclean: clean



