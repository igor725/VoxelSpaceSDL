#ifndef VSMAP_H
#define VSMAP_H
#include "types.h"

int Map_Open(Map *map, const char *diffuse, const char *height);
unsigned char Map_GetHeight(Map *map, Point *p);
void Map_Draw(Map *map, Camera *cam, SDL_Texture *screen);
void Map_Close(Map *map);
#endif
