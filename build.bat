@echo off
setlocal enableextensions enabledelayedexpansion
set USE_SDL_IMAGE=0
set VOXEL_CFLAGS=/MTd /Z7 /FC
set VOXEL_OUT_EXE=vs.exe
set VOXEL_INCLUDES=/ISDL2\include\
set VOXEL_LIBS=%Platform%\SDL2.lib
set VOXEL_LIBPATHS=/libpath:SDL2\lib\

IF "!USE_SDL_IMAGE!"=="1" (
	set VOXEL_INCLUDES=!VOXEL_INCLUDES! /ISDL2_Image\include\
	set VOXEL_LIBPATHS=!VOXEL_LIBPATHS! /libpath:SDL2_image\lib\
	set VOXEL_LIBS=!VOXEL_LIBS! !Platform!\SDL2_image.lib
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /DUSE_SDL_IMAGE
	set PATH=SDL2_Image\lib\!Platform!;!PATH!
)

IF NOT EXIST out MD out

cl %VOXEL_CFLAGS% %VOXEL_INCLUDES% main.c /Foout\ /Feout\%VOXEL_OUT_EXE% /link /SUBSYSTEM:CONSOLE %VOXEL_LIBPATHS% %VOXEL_LIBS%
IF %ERRORLEVEL% NEQ 0 goto error else goto allok

:allok
endlocal
exit 0

:error
endlocal
exit 1
