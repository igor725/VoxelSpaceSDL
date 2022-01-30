#!/bin/bash
if [ "$CC" = "" ]; then
	CC="gcc";
fi
CC="$CC -fdiagnostics-color=always";
VOXEL_OUTDIR="out";
VOXEL_OUTFILE="vs";
VOXEL_CFLAGS="-Isrc/";
VOXEL_SDLIMAGE=0;
VOXEL_SDLTTF=0;
VOXEL_EMSCRIPTEN=0;
VOXEL_THREADED=0;
VOXEL_LIBS=sdl2;
VOXEL_WARN="sign";
VOXEL_OPT="-O2";
VOXEL_DEBUG=0;
VOXEL_RUN=0;

for a in "$@"
do
	if [ "$a" == "sdlimage" ]; then VOXEL_SDLIMAGE=1; fi
	if [ "$a" == "overlay" ]; then VOXEL_SDLTTF=1; fi
	if [ "$a" == "threaded" ]; then VOXEL_THREADED=1; fi
	if [ "$a" == "woff" ]; then VOXEL_WARN="no"; fi
	if [ "$a" == "winf" ]; then VOXEL_WARN="inform"; fi
	if [ "$a" == "wall" ]; then VOXEL_WARN="all"; fi
	if [ "$a" == "web" ]; then VOXEL_EMSCRIPTEN=1; fi
	if [ "$a" == "dbg" ]; then VOXEL_DEBUG=1; fi
	if [ "$a" == "run" ]; then VOXEL_RUN=1; fi
done

if [ $VOXEL_DEBUG -eq 1 ]; then
	VOXEL_CFLAGS="-g $VOXEL_CFLAGS";
	VOXEL_OPT="-O0";
fi

if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
	if [ -z "$EMSDK" ]; then
		echo "EMSDK environment not set";
		exit 1;
	fi
	CC="emcc";
	VOXEL_OUTFILE="index.html";
	VOXEL_OUTDIR="$VOXEL_OUTDIR/html";
	[ -d $VOXEL_OUTDIR ] || mkdir -p $VOXEL_OUTDIR;
	VOXEL_CFLAGS="$VOXEL_CFLAGS -s USE_SDL=2 -s SINGLE_FILE=1";
	VOXEL_CFLAGS="$VOXEL_CFLAGS -s ALLOW_MEMORY_GROWTH=1";
	VOXEL_CFLAGS="$VOXEL_CFLAGS --preload-file out/maps/C1W.bmp@maps/C1W.bmp";
	VOXEL_CFLAGS="$VOXEL_CFLAGS --preload-file out/maps/D1.bmp@maps/D1.bmp";
	if [ $VOXEL_RUN -eq 1 ]; then VOXEL_CFLAGS="$VOXEL_CFLAGS --emrun"; fi
elif [ $VOXEL_THREADED -eq 1 ]; then
	VOXEL_CFLAGS="$VOXEL_CFLAGS -DUSE_THREADED_RENDER"
fi

if [ $VOXEL_SDLIMAGE -eq 1 ]; then
	if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
		VOXEL_CFLAGS="$VOXEL_CFLAGS -s USE_SDL_IMAGE=2";
		VOXEL_CFLAGS="$VOXEL_CFLAGS -s SDL2_IMAGE_FORMATS='[\"png\",\"bmp\"]'";
	else
		VOXEL_LIBS="$VOXEL_LIBS SDL2_image";
	fi
	VOXEL_CFLAGS="$VOXEL_CFLAGS -DUSE_SDL_IMAGE";
fi

if [ $VOXEL_SDLTTF -eq 1 ]; then
	if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
		VOXEL_CFLAGS="$VOXEL_CFLAGS -s USE_SDL_TTF=2";
		VOXEL_CFLAGS="$VOXEL_CFLAGS --preload-file out/Roboto-Regular.ttf@Roboto-Regular.ttf";
	else
		VOXEL_LIBS="$VOXEL_LIBS SDL2_ttf";
	fi
	VOXEL_SOURCES="$VOXEL_SOURCES src/modules/overlay.c"
	VOXEL_CFLAGS="$VOXEL_CFLAGS -DUSE_SDL_TTF";
fi

if [ $VOXEL_EMSCRIPTEN -eq 0 ]; then
	VOXEL_SOURCES="$VOXEL_SOURCES src/modules/dragndrop.c"
	VOXEL_CFLAGS="$VOXEL_CFLAGS $(pkg-config --cflags --libs $VOXEL_LIBS)";
	if [ $? -ne 0 ]; then exit 1; fi
fi

if [ "$VOXEL_WARN" == "no" ]; then
	VOXEL_CFLAGS="$VOXEL_CFLAGS -w";
elif [ "$VOXEL_WARN" == "inform" ]; then
	VOXEL_CFLAGS="$VOXEL_CFLAGS -Wextra";
elif [ "$VOXEL_WARN" == "all" ]; then
	VOXEL_CFLAGS="$VOXEL_CFLAGS -Wall -pedantic";
fi

VOXEL_SOURCES="$(find src/ -maxdepth 1 -type f -iname "*.c" -exec echo -n {} +) $VOXEL_SOURCES";
$CC $VOXEL_SOURCES $VOXEL_OPT -o$VOXEL_OUTDIR/$VOXEL_OUTFILE $VOXEL_CFLAGS;
if [ $? -ne 0 ]; then exit 1; fi

if [ $VOXEL_RUN -eq 1 ]; then
	if [ $VOXEL_EMSCRIPTEN -eq 1 ]; then
		emrun "$VOXEL_OUTDIR/$VOXEL_OUTFILE";
	else
		exec "$VOXEL_OUTDIR/$VOXEL_OUTFILE";
	fi
fi
