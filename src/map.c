#include <SDL_render.h>
#include <SDL_log.h>
#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif
#include "constants.h"
#include "engine.h"
#include "camera.h"
#include "map.h"

static SDL_Surface *ReencodeSurface(SDL_Surface *old) {
	if(old == NULL) return NULL;
	SDL_Surface *new = old;

	if(old->format->format != SDL_PIXELFORMAT_ARGB8888) {
		new = SDL_ConvertSurfaceFormat(old, SDL_PIXELFORMAT_ARGB8888, 0);
		SDL_FreeSurface(old);
	}

	return new;
}

static SDL_Surface *ScaleSurface(SDL_Surface *old, int side) {
	SDL_Surface *new;
	if((new = SDL_CreateRGBSurfaceWithFormat(0, side, side, 32, SDL_PIXELFORMAT_ARGB8888)) == NULL)
		return NULL;

	SDL_Rect rct = {.w = side, .h = side};
	if(SDL_BlitScaled(old, NULL, new, &rct) != 0)
		return NULL;
	SDL_FreeSurface(old);
	
	return new;
}

int Map_Open(Map *map, const char *diffuse, const char *height) {
	SDL_Surface *sDiffuse = NULL, *sHeight = NULL;
	map->redraw = 1;

#ifndef USE_SDL_IMAGE
	sDiffuse = ReencodeSurface(SDL_LoadBMP(diffuse));
	sHeight = ReencodeSurface(SDL_LoadBMP(height));
#else
	sDiffuse = ReencodeSurface(IMG_Load(diffuse));
	sHeight = ReencodeSurface(IMG_Load(height));
#endif

	if(sDiffuse == NULL || sHeight == NULL)
		return ERROR_MAPLOAD_LOAD;

	if((sDiffuse->w != sDiffuse->h) || (sHeight->w != sHeight->h))
		return ERROR_MAPLOAD_IMGSIZE;

	if((sHeight->w & (sHeight->w - 1)) != 0)
		return ERROR_MAPLOAD_WIDTHINVALID;

	if(sHeight->w < sDiffuse->w) {
		if((sHeight = ScaleSurface(sHeight, sDiffuse->w)) == NULL)
			return ERROR_MAPLOAD_SCALE;
	} else if(sDiffuse->w < sHeight->w)
		return ERROR_MAPLOAD_MAPSMISMATCH;

	map->width = sHeight->w;
	map->height = sHeight->h;
	map->shift = (int)(SDL_log10(sHeight->w) / SDL_log10(2));
	map->hiddeny = SDL_calloc(4, GRAPHICS_WIDTH);
	map->color = SDL_calloc(4, sHeight->w * sHeight->h);
	map->altitude = SDL_calloc(1, sHeight->w * sHeight->h);
	if(!map->hiddeny || !map->color || !map->altitude)
		return ERROR_MALLOC_FAIL;

	const unsigned char *datah = sHeight->pixels;
	unsigned int *datac = sDiffuse->pixels;
	for(int i = 0; i < sHeight->w * sHeight->h; i++) {
		map->color[i] = datac[i];
		map->altitude[i] = datah[i << 2];
	}

	/*
		Высвобождаем память, выделенную
		под два surface объекта, так как
		они больше не нужны.
	*/
	SDL_FreeSurface(sDiffuse);
	SDL_FreeSurface(sHeight);
	map->ready = 1;
	return ERROR_OK;
}

unsigned char Map_GetHeight(Map *map, Point *p) {
	if(!map->ready) return 0;
	unsigned int offset = (((int)p->y & (map->width - 1)) << map->shift) + ((int)p->x & (map->height - 1));
	return map->altitude[offset];
}

static void DrawVerticalLine(int *pixels, int pitch, int x, int top, int bottom, int color) {
	// Линия за пределами окна? Низя такое.
	if(top < 0) top = 0;
	if(top > bottom) return;

	int offset = ((top * pitch) + x);
	for(int i = top; i < bottom; i++) {
		pixels[offset] = color;
		offset += pitch;
	}
}

void Map_Draw(Map *map, Camera *cam, void *screen) {
	if(!map->redraw) return;
	int *pixels = NULL, pitch = 0, width = 0, height = 0;
	SDL_LockTexture((SDL_Texture *)screen, NULL, (void **)&pixels, &pitch);
	SDL_QueryTexture((SDL_Texture *)screen, NULL, NULL, &width, &height);
	pitch /= sizeof(int);

	// Заливаем экран одним цветом
	for(int i = 0; i < width * height; i++)
		pixels[i] = 0x9090E0FF;

	if(map->ready) {
		float sinang = SDL_sinf(cam->angle),
		cosang = SDL_cosf(cam->angle);

		for(int i = 0; i < width; i++)
			map->hiddeny[i] = height;

		float deltaz = 1.0f;

		for(float z = 1.0f; z < cam->distance; z += deltaz) {
			Point pLeft = {-cosang * z - sinang * z, sinang * z - cosang * z},
			pRight = {cosang * z - sinang * z, -sinang * z - cosang * z},
			pDelta = {
				(pRight.x - pLeft.x) / (float)width,
				(pRight.y - pLeft.y) / (float)width
			};
			POINT_ADD(pLeft, cam->position);
			for(int i = 0; i < width; i++) {
				int offset = (((int)pLeft.y & (map->width - 1)) << map->shift) + ((int)pLeft.x & (map->height - 1));
				int top = (int)((cam->height - (float)map->altitude[offset]) / z * 240.0f + cam->horizon);
				DrawVerticalLine(pixels, pitch, i, top, map->hiddeny[i], map->color[offset]);
				/*
					Слегка ускоряем рендер путём скрытия
					накладываемых друг на друга частей линий
				*/
				if(top < map->hiddeny[i]) map->hiddeny[i] = (int)top;
				POINT_ADD(pLeft, pDelta);
			}
			deltaz += cam->zstep;
		}
	}

	SDL_UnlockTexture((SDL_Texture *)screen);
	map->redraw = 0;
}

void Map_Close(Map *map) {
	if(map->altitude) {
		SDL_free(map->altitude);
		map->altitude = NULL;
	}
	if(map->hiddeny) {
		SDL_free(map->hiddeny);
		map->hiddeny = NULL;
	}
	if(map->color) {
		SDL_free(map->color);
		map->color = NULL;
	}
	map->ready = 0;
}
