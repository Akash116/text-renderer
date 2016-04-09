CC= g++
CFLAGS= -g -Wall --pedantic `pkg-config --cflags freetype2`
LDFLAGS= `icu-config --ldflags`
LIBS= -lcairo -lharfbuzz -lharfbuzz-icu `pkg-config --libs freetype2`

all: hb-text-renderer

hb-text-renderer: hb-text-renderer.cpp
	$(CC) $< $(CFLAGS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f hb-text-renderer
