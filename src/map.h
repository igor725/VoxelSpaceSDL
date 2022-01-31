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
		SDL_cond *unlockcond; // Условие разблокировки потоков
		struct sMap *self; // Указатель на карту
		Camera *cam; // Указатель на камеру
		int endwork; // Если равно 1, то все разблокированные потоки завершатся
		int *pixels; // Указатель на массив пикселей экрана
		int pitch; // Длина одной строки массива пикселей
	} rgctx; // Глобальный контекст
	int rctxcnt; // Количество потоков рендеринга
	struct sMapRenderCtx {
		SDL_sem *semaphore; // Семафор для ожидания сигнала от потока
		struct SMapRenderGlobCtx *global; // Глобальный контекст, одинаков для всех потоков
		SDL_Thread *self; // Указатель на объект потока
		int start, end; // Начало и конец куска, который отрисовывает поток
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
