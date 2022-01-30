#include "../engine.h"
#include "overlay.h"
#include <SDL_ttf.h>
#include <SDL_log.h>

TTF_Font* font = NULL;
SDL_Surface *tsurf = NULL;
SDL_Color fgcolor = {0, 0, 0};
SDL_Texture *text = NULL;
SDL_Rect txtrect = {0, 0, 0, 0};
int fpstimer = 0;

void Overlay_Init(void *unused) {
	(void)unused;
	if(TTF_Init() < 0) {
		SDL_LogWarn(0, "Failed to init TTF: %s", TTF_GetError());
		return;
	}

	if((font = TTF_OpenFont("Roboto-Regular.ttf", 20)) == NULL) {
		SDL_LogWarn(0, "Failed to load font: %s", TTF_GetError());
		return;
	}
}

void Overlay_Update(void *ptr) {
	if(!font) return;
	char buffer[20];
	int delta = *(int *)ptr;
	if((fpstimer += delta) > 500) {
		if(SDL_snprintf(buffer, 20, "%02.2f (%d ms)", 1000.0f / delta, delta)) {
			tsurf = TTF_RenderText_Blended(font, buffer, fgcolor);
			txtrect.w = tsurf->w, txtrect.h = tsurf->h;
		}
		fpstimer = 0;
	}
}

void Overlay_Draw(void *ptr) {
	if(!font) return;
	SDL_Renderer *render = (SDL_Renderer *)ptr;
	if(tsurf) {
		if(text) SDL_DestroyTexture(text);
		text = SDL_CreateTextureFromSurface(render, tsurf);
		SDL_FreeSurface(tsurf);
		tsurf = NULL;
	}

	SDL_RenderCopy(render, text, NULL, &txtrect);
}

void Overlay_Stop(void *unused) {
	(void)unused;
	if(font) TTF_CloseFont(font);
	if(text) SDL_DestroyTexture(text);
	TTF_Quit();
}
