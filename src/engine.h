#ifndef VSENGINE_H
#define VSENGINE_H
#include "types.h"
#include "map.h"
#include "camera.h"

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
