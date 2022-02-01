#include <SDL_log.h>
#include <SDL_stdinc.h>
#include <SDL_gamecontroller.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
/*
	На данный момент drag'n'drop в emscripten не поддерживается,
	соответственно и подключать его нет особой нужды.
*/
#include "modules/dragndrop.h"
#endif
#ifdef USE_SDL_TTF
#include "modules/overlay.h"
#endif
#include "defines.h"
#include "engine.h"
#include "map.h"
#include "input.h"
#include "argparse.h"

static void AddController(void *ptr) {
	SDL_Log("Found controller: %s", SDL_GameControllerName((SDL_GameController *)ptr));
}

static void FailController(void *ptr) {
	SDL_Log("Failed to open controller #%d: %s", *(Sint32 *)ptr, SDL_GetError());
}

static void RemoveController(void *ptr) {
	SDL_Log("Controller disconnected: %s", SDL_GameControllerName((SDL_GameController *)ptr));
}

int main(int argc, char *argv[]) {
	EngineSettings es = {
		.vsync = 1,
		.width = GRAPHICS_WIDTH,
		.height = GRAPHICS_HEIGHT,
		.diffusemap = "maps/C1W.bmp",
		.heightmap = "maps/D1.bmp",
	};

	if(CommandArgs_Parse(argc, argv, &es))
		return 999;

	Engine_AddListener(LISTEN_ENGINE_UPDATE, Input_Update);
	Engine_AddListener(LISTEN_SDL_EVENT, Input_Event);
#ifndef EMSCRIPTEN
	Engine_AddListener(LISTEN_SDL_WINDOW, DND_Window);
	Engine_AddListener(LISTEN_SDL_EVENT, DND_Event);
#endif
#ifdef USE_SDL_TTF
	Engine_AddListener(LISTEN_ENGINE_START, Overlay_Init);
	Engine_AddListener(LISTEN_ENGINE_UPDATE, Overlay_Update);
	Engine_AddListener(LISTEN_ENGINE_DRAW, Overlay_Draw);
#endif
	Engine_AddListener(LISTEN_CONTROLLER_ADD, AddController);
	Engine_AddListener(LISTEN_CONTROLLER_FAIL, FailController);
	Engine_AddListener(LISTEN_CONTROLLER_DEL, RemoveController);

	int ret;
	if((ret = Engine_Start(&es)) != 0) {
		Engine_End();
		return ret;
	}

#ifdef EMSCRIPTEN
	emscripten_set_main_loop((em_callback_func)Engine_Update, 0, 1);
#else
	while(Engine_Update());
#endif

	Engine_CallListeners(LISTEN_ENGINE_STOP, NULL);
	Engine_End();
	return 0;
}
