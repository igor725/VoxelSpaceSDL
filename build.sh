#!/bin/bash
if [ "$CC" = "" ]; then
	CC="gcc"
fi
CC="$CC -fdiagnostics-color=always"
VOXEL_OUTDIR="out"
VOXEL_OUTFILE="vs"
VOXEL_CFLAGS="-Isrc/"
VOXEL_SOURCES=$(find src/ -name *.c -exec echo -n {} +)
VOXEL_EMSCRIPTEN=0
VOXEL_LIBS=sdl2
VOXEL_DEBUG=0
VOXEL_RUN=0
VOXEL_OPT="-O2"

for a in "$@"
do
	if [ "$a" == "sdlimage" ]; then VOXEL_LIBS="$VOXEL_LIBS SDL2_image"; fi
	if [ "$a" == "web" ]; then VOXEL_EMSCRIPTEN=1; fi
	if [ "$a" == "dbg" ]; then VOXEL_DEBUG=1; fi
	if [ "$a" == "run" ]; then VOXEL_RUN=1; fi
done

if [ $VOXEL_DEBUG -eq 1 ]; then
	VOXEL_CFLAGS="-g $VOXEL_CFLAGS"
	VOXEL_OPT="-O0"
fi

if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
	if [ -z "$EMSDK" ]; then
		echo "EMSDK environment not set";
		exit 1;
	fi
	CC="emcc";
	VOXEL_OUTDIR="$VOXEL_OUTDIR/html";
	VOXEL_OUTFILE="index.html";
	VOXEL_CFLAGS="$VOXEL_CFLAGS -s USE_SDL=2 -s SINGLE_FILE=1";
	VOXEL_CFLAGS="$VOXEL_CFLAGS -s ALLOW_MEMORY_GROWTH=1";
	VOXEL_CFLAGS="$VOXEL_CFLAGS --preload-file out/maps/C1W.bmp@maps/C1W.bmp";
	VOXEL_CFLAGS="$VOXEL_CFLAGS --preload-file out/maps/D1.bmp@maps/D1.bmp";
else
	VOXEL_CFLAGS="$VOXEL_CFLAGS $(pkg-config --cflags --libs $VOXEL_LIBS)";
fi

$CC $VOXEL_SOURCES $VOXEL_OPT -o$VOXEL_OUTDIR/$VOXEL_OUTFILE $VOXEL_CFLAGS

if [ $VOXEL_RUN -eq 1 ]; then
	if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
		emrun --silence_timeout 4 "$VOXEL_OUTDIR/$VOXEL_OUTFILE";
	else
		exec "$VOXEL_OUTDIR/$VOXEL_OUTFILE";
	fi
fi
