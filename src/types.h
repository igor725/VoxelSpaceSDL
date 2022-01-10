#ifndef VSTYPES_H
#define VSTYPES_H
typedef enum eListeners {
	LISTEN_ENGINE_START,
	LISTEN_ENGINE_UPDATE,
	LISTEN_ENGINE_DRAW,
	LISTEN_SDL_WINDOW,
	LISTEN_SDL_EVENT,
	LISTEN_MAX
} Listeners;

typedef struct sPoint {
	float x, y;
} Point;

#define POINT_ADD(a, b) (a).x += (b).x; (a).y += (b).y;
#define POINT_MAKE(_x, _y) {.x=_x, .y=_y}

typedef struct sMap {
	int redraw;
	int ready, shift;
	int width, height;
	int widthp, heightp;
	int *hiddeny, *color;
	unsigned char *altitude;
} Map;

typedef struct sCamera {
	Point position;
	float height, angle;
	float distance, horizon;
	float zstep;
} Camera;
#endif
