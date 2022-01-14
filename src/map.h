#ifndef VSMAP_H
#define VSMAP_H
#include "camera.h"

typedef struct sMap {
	int ready; // Была ли карта успешно загружена
	int redraw; // Нужно ли перерисовывать карту
	int width, height; // Размерность карты
	int shift; // Побитовое смещение ширины карты
	int *hiddeny; // Спрятанные линии
	int *color; // Текстура карты
	unsigned char *altitude; // Высоты карты
} Map;

int Map_Open(Map *map, const char *diffuse, const char *height);
static inline Uint8 Map_GetHeight(Map *map, Point *p) {
	if(!map->ready) return 0;
	unsigned int offset = (((int)p->y & (map->width - 1)) << map->shift) + ((int)p->x & (map->height - 1));
	return map->altitude[offset];
}
void Map_Draw(Map *map, Camera *cam, void *screen);
void Map_Close(Map *map);
#endif
