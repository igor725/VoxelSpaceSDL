#include <stdio.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include "engine.h"
#include "input.h"
#include "dragndrop.h"

static char *smDiffuse = NULL, *smHeight = NULL;

static void LoadMap(void *unused) {
	(void)unused;
	Map *map = NULL;
	Engine_GetObjects(NULL, &map);
	if(smDiffuse && smHeight)
		Map_Open(map, smDiffuse, smHeight);
	else
		Map_Open(map, "maps/C1W.bmp", "maps/D1.bmp");
}

int main(int argc, char *argv[]) {
	if(argc > 2) {
		smDiffuse = argv[1];
		smHeight = argv[2];	
	}

	Engine_AddListener(LISTEN_ENGINE_START, LoadMap);
	Engine_AddListener(LISTEN_ENGINE_UPDATE, Input_Update);
	Engine_AddListener(LISTEN_SDL_WINDOW, DND_Window);
	Engine_AddListener(LISTEN_SDL_EVENT, DND_Event);
	Engine_AddListener(LISTEN_SDL_EVENT, Input_Event);

	int ret;
	if((ret = Engine_Start()) != 0) {
		Engine_End();
		return ret;
	}

#ifdef EMSCRIPTEN
	emscripten_set_main_loop_arg((em_arg_callback_func)Engine_Update, NULL, 0, 1);
#else
	while(Engine_Update());
#endif

	// Очищаемся
	Engine_End();
	return 0;
}