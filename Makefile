CC= g++
CFLAGS= -I /usr/include/freetype2 -g -O0 -Wall --pedantic `freetype-config --cflags`
LDFLAGS= `icu-config --ldflags`
LIBS= -lcairo -lharfbuzz -lharfbuzz-icu `freetype-config --libs`

all: hb-text-renderer

hb-text-renderer: hb-text-renderer.cpp
	$(CC) $< $(CFLAGS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f hb-text-renderer
