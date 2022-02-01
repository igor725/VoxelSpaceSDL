#include "../engine.h"
#include "overlay.h"
#include <SDL_ttf.h>
#include <SDL_log.h>

TTF_Font* font = NULL;
SDL_Color fgcolor = {0, 0, 0};
float overlaytimer = 0;

struct OverlayContext {
	float delta;
	char buffer[48];
	Camera *cam;
	Map *map;
};

struct OverlayLine {
	const char *format;
	int(*update)(struct OverlayLine *self, struct OverlayContext *ctx);
	SDL_Surface *surf;
	SDL_Texture *tex;
	SDL_Rect rect;
	int updated;
};

static int LineOneUpdate(struct OverlayLine *self, struct OverlayContext *ctx) {
	return SDL_snprintf(
		ctx->buffer, 48,
		self->format,
		1000.0f / ctx->delta,
		ctx->delta,
		ctx->cam->distance
	);
}

static int LineTwoUpdate(struct OverlayLine *self, struct OverlayContext *ctx) {
	return SDL_snprintf(
		ctx->buffer, 48,
		self->format,
		ctx->cam->position.x,
		ctx->cam->position.y,
		ctx->cam->height,
		ctx->cam->angle
	);
}

static int LineThreeUpdate(struct OverlayLine *self, struct OverlayContext *ctx) {
		return SDL_snprintf(
		ctx->buffer, 48,
		self->format,
		ctx->map->width,
		ctx->map->height
#ifdef USE_THREADED_RENDER
		, ctx->map->rctxcnt
#endif
	);
}

static struct OverlayLine lines[] = {
	{
		.format = "%.2f (%.2f ms), d: %.0f",
		.update = LineOneUpdate
	},
	{
		.format = "x: %.2f, y: %.2f, h: %.2f, a: %.2f",
		.update = LineTwoUpdate
	},
	{
#ifdef USE_THREADED_RENDER
		.format = "%dx%d | %d",
#else
		.format = "%dx%d",
#endif
		.update = LineThreeUpdate
	}
};

#define LINES_COUNT sizeof(lines) / sizeof(struct OverlayLine)

void Overlay_Init(void *unused) {
	(void)unused;
	if(TTF_Init() < 0) {
		SDL_LogWarn(0, "Failed to init TTF: %s", TTF_GetError());
		return;
	}

	if((font = TTF_OpenFont("Roboto-Regular.ttf", 16)) == NULL) {
		SDL_LogWarn(0, "Failed to load font: %s", TTF_GetError());
		return;
	}
}

void Overlay_Update(void *ptr) {
	if(!font) return;
	char buffer[32];
	struct OverlayContext ctx = {
		.delta = *(float *)ptr
	};

	if((overlaytimer += ctx.delta) > 500.0f) {
		Engine_GetObjects(&ctx.cam, &ctx.map);
		for(int i = 0; i < LINES_COUNT; i++) {
			struct OverlayLine *line = &lines[i];
			if(line->surf) continue;

			if(line->update) {
				if(line->update(line, &ctx) > 0)
					line->surf = TTF_RenderText_Blended(font, ctx.buffer, fgcolor);
			} else if(!line->tex)
				line->surf = TTF_RenderText_Blended(font, line->format, fgcolor);

			if(line->surf) {
				line->rect.w = line->surf->w, line->rect.h = line->surf->h;
				line->rect.y = i * line->surf->h;
			}
		}
		overlaytimer = 0;
	}
}

void Overlay_Draw(void *ptr) {
	if(!font) return;
	SDL_Renderer *renderer = (SDL_Renderer *)ptr;
	for(int i = 0; i < LINES_COUNT; i++) {
		struct OverlayLine *line = &lines[i];
		if(line->surf) {
			if(line->tex) SDL_DestroyTexture(line->tex);
			line->tex = SDL_CreateTextureFromSurface(renderer, line->surf);
			SDL_FreeSurface(line->surf);
			line->surf = NULL;
		}

		SDL_RenderCopy(renderer, line->tex, NULL, &line->rect);
	}
}

void Overlay_Stop(void *unused) {
	(void)unused;
	if(font) TTF_CloseFont(font);
	for(int i = 0; i < LINES_COUNT; i++) {
		struct OverlayLine *line = &lines[i];
		if(line->surf) SDL_free(line->surf);
		if(line->tex) SDL_free(line->tex);
	}
	TTF_Quit();
}
