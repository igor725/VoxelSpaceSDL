#include "SDL_video.h"
#include "SDL_render.h"
#include "SDL_gamecontroller.h"

#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif

#ifndef min
#define min(a, b) (((a)<(b))?(a):(b))
#define max(a, b) (((a)>(b))?(a):(b))
#endif

typedef struct sPoint {
	float x, y;
} Point;

struct sMap {
	int ready, shift;
	int width, height;
	int widthp, heightp;
	int *hiddeny, *color;
	unsigned char *altitude;
};

struct sCamera {
	Point position;
	float height, angle;
	float distance, horizon;
	float ihorizon;
};

struct sContext {
	float zstep;
	int running, redrawMap;
	int deltaTime, mouseGrabbed;
	int width, height;
	char droppedFileType;
	char *droppedFile;
	SDL_Window *wnd;
	SDL_Renderer *render;
	SDL_Texture *screen;
	SDL_GameController *controller;
	struct sCamera camera;
	struct sMap map;
};

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT (WINDOW_WIDTH * 0.75)
#define MOUSE_SENS 0.057f
#define CAMERA_HORIZON ((float)WINDOW_HEIGHT * 0.50f)
#define WINDOW_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
#define RENDERER_FLAGS SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
