#!/bin/bash
if [ "$CC" = "" ]; then
	CC="gcc"
fi
CC="$CC -fdiagnostics-color=always"
VOXEL_CFLAGS="-oout/vs -lm"
VOXEL_LIBS=sdl2
VOXEL_DEBUG=0
VOXEL_OPT="-O2"

for a in "$@"
do
	if [ "$a" == "sdlimage" ]; then VOXEL_LIBS="$VOXEL_LIBS SDL2_image"; fi
	if [ "$a" == "dbg" ]; then VOXEL_DEBUG=1; fi
done

if [ $VOXEL_DEBUG -eq 1 ]; then
	VOXEL_CFLAGS="-g $VOXEL_CFLAGS"
	VOXEL_OPT="-O0"
fi

$CC main.c $VOXEL_OPT $VOXEL_CFLAGS $(pkg-config --cflags --libs $VOXEL_LIBS)
