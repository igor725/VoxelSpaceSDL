#ifndef VSENGINE_H
#define VSENGINE_H
// Говорим SDL, что не нужно переписывать main
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_gamecontroller.h>
#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif
#include "types.h"
#include "map.h"
#include "camera.h"

int Engine_Start(void);
int Engine_Update(void);
void Engine_Stop(void);
void Engine_End(void);

SDL_Window *Engine_GetWindow(void);
void Engine_GetObjects(Camera **cam, Map **map);
int Engine_GetDeltaTime(void);
void Engine_AddListener(Listeners type, void(*func)(void *));
void Engine_CallListeners(Listeners type, void *arg);
#endif
