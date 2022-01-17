#ifndef VSENGINE_H
#define VSENGINE_H
// Говорим SDL, что не нужно переписывать main
#define SDL_MAIN_HANDLED
typedef struct sPoint {
	float x, y;
} Point;

#include "map.h"
#include "camera.h"

typedef enum eListeners {
	LISTEN_ENGINE_START, // Вызвать при запуске движка
	LISTEN_ENGINE_UPDATE, // Вызывать каждый тик
	LISTEN_ENGINE_DRAW, // Вызывать перед отправкой данных рендереру
	LISTEN_ENGINE_STOP, // Вызвать при остановке движка
	LISTEN_CONTROLLER_FAIL, // Вызвать при ошибке чтения геймпада
	LISTEN_CONTROLLER_ADD, // Вызвать при подключении геймпада к компьютеру
	LISTEN_CONTROLLER_DEL, // Вызвать при отключении геймпада от компьютера
	LISTEN_SDL_WINDOW, // Вызвать при создании SDL окна
	LISTEN_SDL_EVENT, // Вызывать при получении события от SDL
	LISTEN_TYPES_MAX
} Listeners;


int Engine_Start(void);
int Engine_Update(void);
void Engine_Stop(void);
void Engine_End(void);

void *Engine_GetWindow(void);
void Engine_GetObjects(Camera **cam, Map **map);
int Engine_GetDeltaTime(void);
void Engine_AddListener(Listeners type, void(*func)(void *));
void Engine_CallListeners(Listeners type, void *arg);
#endif
