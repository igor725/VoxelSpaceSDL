#ifndef VSMAP_H
#define VSMAP_H
#include "camera.h"
#ifdef USE_THREADED_RENDER
#include <SDL_thread.h>
#endif

typedef struct sMap {
	int ready; // Была ли карта успешно загружена
	int redraw; // Нужно ли перерисовывать карту
	int width, height; // Размерность карты
	int shift; // Побитовое смещение ширины карты
	int optimize; // Оптимизировать отрисовку
	float optdist; // Дистанция урезания качества
	int *hiddeny; // Спрятанные линии
	int *color; // Текстура карты
	unsigned char *altitude; // Высоты карты
	void *screen; // Экран отрисовки карты

#ifdef USE_THREADED_RENDER
	struct SMapRenderGlobCtx {
		SDL_cond *unlockcond;
		struct sMap *self;
		Camera *cam;
		int endwork;
		int fullwidth;
		int *pixels;
		int pitch;
	} rgctx;
	int rctxcnt;
	struct sMapRenderCtx {
		SDL_sem *semaphore;
		struct SMapRenderGlobCtx *global;
		SDL_Thread *self;
		int start, end;
	} *rctxs;
#endif
} Map;

void Map_SetScreen(Map *map, void *screen);
int Map_Open(Map *map, const char *diffuse, const char *height);
static inline Uint8 Map_GetHeight(Map *map, Point *p) {
	if(!map->ready) return 0;
	unsigned int offset = (((int)p->y & (map->width - 1)) << map->shift) + ((int)p->x & (map->height - 1));
	return map->altitude[offset];
}
void Map_Draw(Map *map, Camera *cam);
void Map_Close(Map *map);
#endif
