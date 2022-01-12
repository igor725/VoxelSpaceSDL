// Говорим SDL, что не нужно переписывать main
#define SDL_MAIN_HANDLED
#include <SDL.h>
#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif
#include "constants.h"
#include "engine.h"
#include "map.h"

struct sContext {
	unsigned int lastTime, currTime;
	int deltaTime, stopped;
	SDL_Window *wnd;
	SDL_Renderer *render;
	SDL_Texture *screen;
	struct sListener {
		void(*func)(void *);
		struct sListener *next;
	} *listeners[LISTEN_MAX];
	Camera camera;
	Map map;
} ctx = {
	.camera = {
		.zstep = CAMERA_ZSTEP_DEFAULT,
		.position = CAMERA_POSITION_DEFAULT,
		.height = CAMERA_HEIGHT_DEFAULT,
		.horizon = CAMERA_HORIZON_DEFAULT,
		.distance = CAMERA_DISTANCE_DEFAULT
	}
};

static int CreateSDLWindow(unsigned int flags) {
	if((ctx.wnd = SDL_CreateWindow("VoxelSpace SDL",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		GRAPHICS_WIDTH, GRAPHICS_HEIGHT, flags
	)) == NULL) return 0;
	Engine_CallListeners(LISTEN_SDL_WINDOW, ctx.wnd);
	return 1;
}

static void CompareSDLVersions(const char *libname, const SDL_version *cv, const SDL_version *lv) {
	if(SDL_VERSIONNUM(cv->major, cv->minor, cv->patch) != SDL_VERSIONNUM(lv->major, lv->minor, lv->patch)) {
		SDL_LogWarn(0, "%s library version mismatch. Expected: %d.%d.%d, loaded: %d.%d.%d",
			libname, cv->major, cv->minor, cv->patch, lv->major, lv->minor, lv->patch
		);
	} else SDL_Log("%s library version: %d.%d.%d", libname, cv->major, cv->minor, cv->patch);
}

int Engine_Start(void) {
	SDL_SetMainReady();
	// Инициализируем SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
		if(SDL_Init(SDL_INIT_VIDEO) != 0) {
			SDL_LogError(0, "SDL_Init failed: %s", SDL_GetError());
			return 1;
		}
	}

	SDL_version cver, lver;
	SDL_GetVersion(&lver);
	SDL_VERSION(&cver);
	CompareSDLVersions("SDL", &cver, &lver);

#ifdef USE_SDL_IMAGE
	SDL_IMAGE_VERSION(&cver);
	CompareSDLVersions("SDL Image", &cver, IMG_Linked_Version());
#endif


	if(!CreateSDLWindow(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) {
		SDL_LogError(0, "Failed to create SDL window: %s", SDL_GetError());
		return 2;
	}

	/*
		Ищем у пользователя рендерер, подходящий
		под наши рендерерские запросы.
	*/
	if((ctx.render = SDL_CreateRenderer(ctx.wnd, -1, GRAPHICS_FLAGS)) == NULL) {
		SDL_LogError(0, "Failed to create SDL renderer: %s", SDL_GetError());
		return 3;
	} else {
		SDL_RendererInfo ri;
		if(SDL_GetRendererInfo(ctx.render, &ri) == 0)
			SDL_Log("Using %s %s renderer", ri.flags & SDL_RENDERER_ACCELERATED ? "hardware" : "software", ri.name);
		else
			SDL_LogError(0, "Failed to retrieve SDL renderer info: %s", SDL_GetError());
	}

	/*
		Создаём текстуру, которая будет использоваться
		для хранения последнего отрисованного кадра.
	*/
	if((ctx.screen = SDL_CreateTexture(ctx.render,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		GRAPHICS_WIDTH, GRAPHICS_HEIGHT
	)) == NULL) {
		SDL_LogError(0, "Failed to create SDL texture: %s", SDL_GetError());
		return 4;
	}

	Engine_CallListeners(LISTEN_ENGINE_START, NULL);
	return 0;
}

void Engine_AddListener(Listeners type, void(*func)(void *)) {
	struct sListener *newlistener = SDL_calloc(1, sizeof(struct sListener));
	if(ctx.listeners[type]) newlistener->next = ctx.listeners[type];
	ctx.listeners[type] = newlistener;
	newlistener->func = func;
}

void Engine_CallListeners(Listeners type, void *arg) {
	struct sListener *listener = ctx.listeners[type];

	while(listener) {
		listener->func(arg);
		listener = listener->next;
	}
}

int Engine_Update(void) {
	if(ctx.stopped) return 0;
	// Обрабатываем накопившиеся эвенты SDL
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
			case SDL_QUIT:
				return 0;
			default:
				Engine_CallListeners(LISTEN_SDL_EVENT, &ev);
				break;
		}
	}

	// Выполняем все функции, ждущие события UPDATE
	Engine_CallListeners(LISTEN_ENGINE_UPDATE, &ctx.deltaTime);

	// Перерисовываем мир
	Map_Draw(&ctx.map, &ctx.camera, ctx.screen);

	// Рисуем в SDL окне нашу текстуру
	SDL_RenderClear(ctx.render);
	Engine_CallListeners(LISTEN_ENGINE_DRAW, ctx.screen);
	SDL_RenderCopy(ctx.render, ctx.screen, NULL, NULL);
	SDL_RenderPresent(ctx.render);

	// Считаем время, затраченное на полный тик
	ctx.lastTime = ctx.currTime;
	ctx.currTime = SDL_GetTicks();
	ctx.deltaTime = ctx.currTime - ctx.lastTime;
	return 1;
}

void *Engine_GetWindow(void) {
	return ctx.wnd;
}

void Engine_GetObjects(Camera **cam, Map **map) {
	if(cam) *cam = &ctx.camera;
	if(map) *map = &ctx.map;
}

int Engine_GetDeltaTime(void) {
	return ctx.deltaTime;
}

void Engine_Stop(void) {
	ctx.stopped = 1;
}

void Engine_End(void) {
	if(ctx.wnd) {
		SDL_DestroyWindow(ctx.wnd);
		ctx.wnd = NULL;
	}
	if(ctx.render) {
		SDL_DestroyRenderer(ctx.render);
		ctx.render = NULL;
	}
	if(ctx.screen) {
		SDL_DestroyTexture(ctx.screen);
		ctx.screen = NULL;
	}

	Map_Close(&ctx.map);
	SDL_Quit();
}
