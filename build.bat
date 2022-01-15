@echo off
setlocal enableextensions enabledelayedexpansion
set VOXEL_USE_SDLIMAGE=0
set VOXEL_OUTDIR=out
set VOXEL_OUTFILE=vs.exe
set VOXEL_EMSCRIPTEN=0
set VOXEL_RUN=0
set VOXEL_OPT=no
set VOXEL_DEBUG=0
set VOXEL_USE_SDLTTF=0
set VOXEL_WARN=sign

:argloop
IF "%1"=="" goto argfin
IF "%1"=="sdlimage" set VOXEL_USE_SDLIMAGE=1
IF "%1"=="overlay" set VOXEL_USE_SDLTTF=1
IF "%1"=="dbg" set VOXEL_DEBUG=1
IF "%1"=="web" set VOXEL_EMSCRIPTEN=1
IF "%1"=="optspeed" set VOXEL_OPT=speed
IF "%1"=="optsize" set VOXEL_OPT=size
IF "%1"=="woff" set VOXEL_WARN=no
IF "%1"=="winf" set VOXEL_WARN=inform
IF "%1"=="wall" set VOXEL_WARN=all
IF "%1"=="run" set VOXEL_RUN=1
shift
GOTO argloop
:argfin

IF "%VOXEL_EMSCRIPTEN%"=="1" (
	GOTO web
) else (
	GOTO mscl
)

:mscl
FOR /F "tokens=* USEBACKQ" %%F IN (`dir /b /a-d src\*.c`) DO (
	set VOXEL_SOURCES=!VOXEL_SOURCES!src\%%F 
)
set VOXEL_SOURCES=%VOXEL_SOURCES% src\modules\dragndrop.c

IF NOT EXIST %VOXEL_OUTDIR% MD !VOXEL_OUTDIR!
set VOXEL_CFLAGS=/FC /Isrc\ /Fo%VOXEL_OUTDIR%\ /Fe%VOXEL_OUTDIR%\%VOXEL_OUTFILE% /ISDL2\include\
set VOXEL_LINK=/SUBSYSTEM:CONSOLE /libpath:SDL2\lib\%VSCMD_ARG_TGT_ARCH%
set PATH=SDL2\lib\%VSCMD_ARG_TGT_ARCH%;!PATH!
set VOXEL_LIBS=SDL2.lib

IF "%VOXEL_USE_SDLIMAGE%"=="1" (
	set VOXEL_LIBS=!VOXEL_LIBS! SDL2_image.lib
	set PATH=SDL2_image\lib\!VSCMD_ARG_TGT_ARCH!;!PATH!
	set VOXEL_LINK=!VOXEL_LINK! /libpath:SDL2_image\lib\!VSCMD_ARG_TGT_ARCH!
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /ISDL2_Image\include\
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /DUSE_SDL_IMAGE
)

IF "%VOXEL_USE_SDLTTF%"=="1" (
	set VOXEL_LIBS=!VOXEL_LIBS! SDL2_ttf.lib
	set PATH=SDL2_ttf\lib\!VSCMD_ARG_TGT_ARCH!;!PATH!
	set VOXEL_LINK=!VOXEL_LINK! /libpath:SDL2_ttf\lib\!VSCMD_ARG_TGT_ARCH!
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /ISDL2_ttf\include\
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /DUSE_SDL_TTF
	set VOXEL_SOURCES=%VOXEL_SOURCES% src/modules/overlay.c
)

IF "%VOXEL_DEBUG%"=="1" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /MTd /Z7 /DDEBUG
	set VOXEL_OPT=/Od
) ELSE (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /MT
	set VOXEL_LINK=!VOXEL_LINK! /RELEASE
)

IF "%VOXEL_WARN%"=="no" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /W0
) ELSE IF "%VOXEL_WARN%"=="sign" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /W2
) ELSE IF "%VOXEL_WARN%"=="inform" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /W4
) ELSE IF "%VOXEL_WARN%"=="all" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /Wall
)

IF "!VOXEL_OPT!" == "no" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /Od
) ELSE IF "!VOXEL_OPT!" == "speed" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /O1
) ELSE IF "!VOXEL_OPT!" == "size" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! /O2
)

cl %VOXEL_CFLAGS% %VOXEL_SOURCES% ^
/link %VOXEL_LINK% %VOXEL_LIBS%
IF NOT "%ERRORLEVEL%"=="0" (GOTO error) ELSE (GOTO allok)

:web
IF "%EMSDK%"=="" (
	echo EMSDK environment not set
	EXIT /B 1
)

FOR /F "tokens=* USEBACKQ" %%F IN (`dir /b /a-d src\*.c`) DO (
	set VOXEL_SOURCES=!VOXEL_SOURCES! src/%%F
)

set VOXEL_CFLAGS=-s SINGLE_FILE=1
set VOXEL_CFLAGS=%VOXEL_CFLAGS% -s ALLOW_MEMORY_GROWTH=1
set VOXEL_CFLAGS=%VOXEL_CFLAGS% -s USE_SDL=2
set VOXEL_CFLAGS=%VOXEL_CFLAGS% --preload-file out/maps/C1W.bmp@maps/C1W.bmp
set VOXEL_CFLAGS=%VOXEL_CFLAGS% --preload-file out/maps/D1.bmp@maps/D1.bmp
set VOXEL_OUTDIR=%VOXEL_OUTDIR%/html
set VOXEL_OUTFILE=index.html

IF NOT EXIST %VOXEL_OUTDIR% MD !VOXEL_OUTDIR!

IF "%VOXEL_RUN%"=="1" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! --emrun
)

IF "%VOXEL_DEBUG%"=="1" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -g
	set VOXEL_OPT=no
)

IF "%VOXEL_USE_SDLIMAGE%"=="1" (
	echo WARN: SDL2_image doesn't work well with emscripten
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -s USE_SDL_IMAGE=2 -DUSE_SDL_IMAGE
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -s SDL2_IMAGE_FORMATS="[""bmp"",""png""]"
)

IF "%VOXEL_USE_SDLTTF%"=="1" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -s USE_SDL_TTF=2 -DUSE_SDL_TTF
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! --preload-file out/Roboto-Regular.ttf@Roboto-Regular.ttf
	set VOXEL_SOURCES=!VOXEL_SOURCES! src/modules/overlay.c
)

IF "%VOXEL_OPT%" == "no" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -O0
) ELSE IF "%VOXEL_OPT%" == "speed" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -O1
) ELSE IF "%VOXEL_OPT%" == "size" (
	set VOXEL_CFLAGS=!VOXEL_CFLAGS! -O2
)

CALL emcc%VOXEL_SOURCES% %VOXEL_CFLAGS% -o%VOXEL_OUTDIR%/%VOXEL_OUTFILE%
IF "%ERRORLEVEL%"=="0" (GOTO allok) ELSE (GOTO error)

:error
endlocal
EXIT /B 1

:allok
IF "%VOXEL_RUN%"=="1" (
	IF "%VOXEL_EMSCRIPTEN%"=="1" (
		CALL emrun !VOXEL_OUTDIR!/!VOXEL_OUTFILE!
	) ELSE (
		%VOXEL_OUTDIR%\%VOXEL_OUTFILE%
	)
)

endlocal
EXIT /B 0
