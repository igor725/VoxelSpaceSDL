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
#include "constants.h"
#include "engine.h"
#define VSERROR_STRINGS
#include "error.h"
#include "map.h"
#include "input.h"

#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#else
#include <unistd.h>
#endif

static char *smDiffuse = NULL, *smHeight = NULL;

static void LoadMap(void *unused) {
	(void)unused;
	Map *map = NULL;
	Engine_GetObjects(NULL, &map);
	if(!smDiffuse || !smHeight)
		smDiffuse = "maps/C1W.bmp", smHeight = "maps/D1.bmp";

	Errors ret;
	if((ret = Map_Open(map, smDiffuse, smHeight)) != ERROR_OK)
		SDL_LogError(0, "Failed to load map: %s", Errors_Strings[ret]);
}

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
	char *pathend = SDL_strrchr(argv[0], '\\');
	if(!pathend) pathend = SDL_strrchr(argv[0], '/');
	if(pathend) {
		*pathend = '\0';
		if(chdir(argv[0]) == -1)
			SDL_LogWarn(0, "Failed to change directory");
	}
	
	if(argc > 2) {
		smDiffuse = argv[1];
		smHeight = argv[2];	
	}

	Engine_AddListener(LISTEN_ENGINE_START, LoadMap);
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
	if((ret = Engine_Start()) != 0) {
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
