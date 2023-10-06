#include <SDL_events.h>
#include <SDL_stdinc.h>
#include "defines.h"
#include "engine.h"
#include "camera.h"
#include "map.h"
#include "input.h"

int isGravitationEnabled = 0, isOnTheGround = 0;
float velocity = 0.0f;

#include "input/kbmouse.c"
#include "input/gamepad.c"

void Input_Update(void *ptr) {
	float delta = *(float *)ptr;
	Map *map = NULL;
	Camera *cam = NULL;
	Engine_GetObjects(&cam, &map);

	// Запрашиваем перерисовку мир, если была нажата какая-либо кнопка
	if(PollControllers(cam, delta * 0.03f) || ProcessKeyboard(cam, delta * 0.03f))
		map->redraw = 1;

	float minHeight = (float)Map_GetHeight(map, &cam->position);
	if(isGravitationEnabled) {
		minHeight += 9.0f;
		if(!isOnTheGround) {
			velocity -= delta * INPUT_GRAVITATION_MULT * 0.001f;
			cam->height += velocity;
		}

		if(cam->height > minHeight) {
			isOnTheGround = 0;
			map->redraw = 1;
		} else if(!isOnTheGround) {
			cam->height = minHeight;
			isOnTheGround = 1;
			velocity = 0.0f;
			map->redraw = 1;
		}
	} else minHeight += 2.0f;

	// Вытаскиваем камеру из-под земли, если она там
	cam->height = max(minHeight, min(cam->height, CAMERA_HEIGHT_MAX));
	// Обнуляем угл камеры, если она прошла полный круг
	if(SDL_fabsf(cam->angle) > M_PI * 2) cam->angle = 0;
}

void Input_Event(void *ptr) {
	SDL_Event *ev = (SDL_Event *)ptr;
	switch(ev->type) {
		case SDL_CONTROLLERDEVICEADDED:
			AddController(ev->cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			RemoveController(ev->cdevice.which);
			break;
		case SDL_KEYDOWN:
			ProcessKeyDown(ev->key.keysym.scancode, ev->key.keysym.mod);
			break;
		case SDL_KEYUP:
			ProcessKeyUp(ev->key.keysym.scancode);
			break;
		case SDL_MOUSEBUTTONUP:
			persistGrab = ev->button.clicks > 1;
			if(persistGrab) isMouseGrabbed = 0;
		case SDL_MOUSEBUTTONDOWN:
			if(ev->button.button == SDL_BUTTON_LEFT)
				ToggleMouseGrab();
			break;
		case SDL_MOUSEMOTION:
			if(isMouseGrabbed)
				ProcessMouseMotion(ev->motion.xrel, ev->motion.yrel);
			break;
	}
}
