#include "SDL_video.h"
#include "SDL_render.h"

typedef struct sPoint {
	float x, y;
} Point;

struct sContext {
	float zstep;
	int running, redrawMap;
	int deltaTime, mouseGrabbed;
	char droppedFileType;
	char *droppedFile;
	SDL_Window *wnd;
	SDL_Renderer *render;
	SDL_Texture *screen;
	struct sCamera {
		Point position;
		float height, angle;
		float distance, horizon;
		float ihorizon;
	} camera;
	struct sMap {
		int ready, shift;
		int width, height;
		int widthp, heightp;
		int *hiddeny, *color;
		unsigned char *altitude;
	} map;
};

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT (WINDOW_WIDTH * 0.75)
#define MOUSE_SENS 0.057f
#define CAMERA_HORIZON ((float)WINDOW_HEIGHT * 0.50f)
#define WINDOW_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
#define RENDERER_FLAGS SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE