@echo off
setlocal enableextensions enabledelayedexpansion
set VOXEL_USE_SDLIMAGE=0
set VOXEL_CFLAGS=/FC
set VOXEL_OUT_EXE=vs.exe
set VOXEL_INCLUDES=/ISDL2\include\
set VOXEL_LIBS=SDL2.lib
set VOXEL_LIBPATHS=/libpath:SDL2\lib\%VSCMD_ARG_TGT_ARCH%
set VOXEL_OPT=/Od

:argloop
IF "%1"=="" goto argfin
IF "%1"=="sdlimage" set VOXEL_USE_SDLIMAGE=1
IF "%1"=="dbg" set VOXEL_DEBUG=1
IF "%1"=="optspeed" set VOXEL_OPT=/O2 /Ox
IF "%1"=="optsize" set VOXEL_OPT=/O1
SHIFT
goto argloop

:argfin
IF "!VOXEL_USE_SDLIMAGE!"=="1" (
	set VOXEL_INCLUDES=!VOXEL_INCLUDES! /ISDL2_Image\include\
	set VOXEL_LIBPATHS=!VOXEL_LIBPATHS! /libpath:SDL2_image\lib\!VSCMD_ARG_TGT_ARCH!
	set VOXEL_LIBS=!VOXEL_LIBS! SDL2_image.lib
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /DUSE_SDL_IMAGE
	set PATH=SDL2_Image\lib\!VSCMD_ARG_TGT_ARCH!;!PATH!
)

IF "%VOXEL_DEBUG%"=="1" (
	set VOXEL_CFLAGS=%VOXEL_CFLAGS% /MTd /Z7
	set VOXEL_OPT=/Od
) else (
	set VOXEL_CFLAGS=%VOXEL_CFLAGS% /MT
)

IF NOT EXIST out MD out

cl %VOXEL_CFLAGS% %VOXEL_OPT% %VOXEL_INCLUDES% src/*.c /Isrc\ /Foout\ ^
/Feout\%VOXEL_OUT_EXE% /link /SUBSYSTEM:CONSOLE %VOXEL_LIBPATHS% %VOXEL_LIBS%
IF %ERRORLEVEL% NEQ 0 goto error else goto allok

:allok
endlocal
exit /b 0

:error
endlocal
exit /b 1
