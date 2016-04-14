CXXFLAGS= -g -Wall --pedantic `pkg-config --cflags cairo harfbuzz harfbuzz-icu icu-uc freetype2`
LDFLAGS= `pkg-config --libs icu-uc`
LIBS= `pkg-config --libs cairo harfbuzz harfbuzz-icu icu-uc freetype2`

all: hb-text-renderer

hb-text-renderer: hb-text-renderer.cpp
	$(CXX) $< $(CXXFLAGS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f hb-text-renderer
