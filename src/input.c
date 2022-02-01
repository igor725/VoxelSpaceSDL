#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>
#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_version.h>
#include <SDL_log.h>
#include "defines.h"
#include "engine.h"
#include "camera.h"
#include "map.h"
#include "input.h"

static SDL_GameController *pads[INPUT_MAX_PADS] = {0};
static int padButtonState[SDL_CONTROLLER_BUTTON_MAX] = {0};
static SDL_Scancode input[INPUT_MAX_KEYBINDS] = {0};
static int isMouseGrabbed = 0, persistGrab = 0,
isGravitationEnabled = 0, isOnTheGround = 0;
static float velocity = 0.0f;

static inline void AddController(Sint32 id) {
	if(id < INPUT_MAX_PADS && SDL_IsGameController(id)) {
		SDL_GameController *pad = SDL_GameControllerOpen(id);
		if(pad) {
			pads[id] = pad;
			Engine_CallListeners(LISTEN_CONTROLLER_ADD, pad);
		} else Engine_CallListeners(LISTEN_CONTROLLER_FAIL, &id);
	}
}

static inline void RemoveController(Sint32 id) {
	if(id < INPUT_MAX_PADS && pads[id]) {
		Engine_CallListeners(LISTEN_CONTROLLER_DEL, pads[id]);
		SDL_GameControllerClose(pads[id]);
		pads[id] = NULL;
	}
}

static int ProcessControllerButtonDown(Camera *cam, SDL_GameControllerButton btn) {
	switch(btn) {
		case SDL_CONTROLLER_BUTTON_B:
			Engine_Stop();
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			Camera_ResetDistance(cam);
			return 1;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			Camera_AdjustDistance(cam, (btn == SDL_CONTROLLER_BUTTON_LEFTSHOULDER ? -1.0f : 1.0f) * CAMERA_DISTANCE_STEP);
			return 1;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			Camera_ResetPitch(cam);
			return 1;
		case SDL_CONTROLLER_BUTTON_BACK:
			isGravitationEnabled = !isGravitationEnabled;
			return 1;
		default: break;
	}

	return 0;
}

static int ProcessControllerButtonHold(Camera *cam, SDL_GameControllerButton btn, float dm) {
	switch (btn) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			if(!isGravitationEnabled)
				Camera_StrafeVert(cam, (btn == SDL_CONTROLLER_BUTTON_DPAD_UP ? dm : -dm));
			return 1;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			Camera_StrafeHoriz(cam, (btn == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? -dm : dm));
			return 1;
		case SDL_CONTROLLER_BUTTON_A:
			if(isGravitationEnabled && isOnTheGround) {
				velocity += INPUT_JUMP_VELOCITY;
				isOnTheGround = 0;
			}
			return 1;

		default: break;
	}

	return 0;
}

static int PollController(SDL_GameController *pad, Camera *cam, float dm) {
	Point leftStick = {
		.x = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f,
		.y = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f
	};

	Point rightStick = {
		.x = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f,
		.y = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f
	};

	Point trigger = {
		.x = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f,
		.y = (float)SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f
	};

	int handled = 0;

	if(SDL_fabsf(leftStick.x) > 0.15f || SDL_fabsf(leftStick.y) > 0.15f) {
		float speedup = 1.0f + trigger.x;
		if(isGravitationEnabled) speedup /= 3.0f;
		Camera_MoveForward(cam, leftStick.y * speedup * dm, isGravitationEnabled);
		Camera_StrafeHoriz(cam, leftStick.x * speedup * dm);
		handled = 1;
	}

	if(SDL_fabsf(rightStick.x) > 0.15f || SDL_fabsf(rightStick.y) > 0.15f) {
		float speedup = 1.0f + trigger.y;
		Camera_Rotate(cam, rightStick.x * speedup * dm);
		Camera_Pitch(cam, rightStick.y * speedup * dm);
		handled = 1;
	}

	for(SDL_GameControllerButton i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
		if(SDL_GameControllerGetButton(pad, i)) {
			if(!padButtonState[i]) {
				if(ProcessControllerButtonDown(cam, i)) handled = 1;
				padButtonState[i] = 1;
			} else if(ProcessControllerButtonHold(cam, i, dm)) handled = 1;
		} else if(padButtonState[i]) padButtonState[i] = 0;
	}

#if SDL_VERSION_ATLEAST(2, 0, 14)
	if(SDL_GameControllerGetNumTouchpads(pad) > 0) {
		Uint8 state = 0;
		static float ox = 0.0f, oy = 0.0f;
		float x = 0.0f, y = 0.0f, dx = 0.0f, dy = 0.0f;
		if(SDL_GameControllerGetTouchpadFinger(pad, 0, 0, &state, &x, &y, NULL) == 0) {
			if(state == 1) {
				if(ox == 0.0f && oy == 0.0f)
					ox = x, oy = y;
				dx = x - ox, dy = y - oy;
				Camera_Rotate(cam, dx * dm * INPUT_TOUCH_SENS);
				Camera_Pitch(cam, dy * dm * INPUT_TOUCH_SENS);
				ox = x, oy = y;
				handled = 1;
			} else ox = 0.0f, oy = 0.0f;
		}
	}
#endif

	return handled;
}
static int PollControllers(Camera *cam, float dm) {
	for(Sint32 id = 0; id < INPUT_MAX_PADS; id++) {
		SDL_GameController *pad = pads[id];
		if(pad && PollController(pad, cam, dm)) return 1;
	}

	return 0;
}

static inline void ToggleMouseGrab(void) {
	isMouseGrabbed = !isMouseGrabbed || persistGrab;
	SDL_SetRelativeMouseMode(isMouseGrabbed);
	SDL_SetWindowGrab(Engine_GetWindow(), isMouseGrabbed);
}

static void ProcessKeyDown(SDL_Scancode code, Uint16 mod) {
	Camera *cam = NULL;
	Map *map = NULL;
	Engine_GetObjects(&cam, &map);
	switch(code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_S:
			input[0] = code;
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_D:
			input[1] = code;
			break;
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_Q:
			input[2] = code;
			break;
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_F:
			input[3] = code;
			break;
		case SDL_SCANCODE_SPACE:
			map->redraw = 1;
			if(mod & KMOD_CTRL)
				Camera_ResetDistance(cam);
			else
				Camera_AdjustDistance(cam, (mod & KMOD_SHIFT ? -1.0f : 1.0f) * CAMERA_DISTANCE_STEP);
			break;
		case SDL_SCANCODE_C:
			Camera_ResetPitch(cam);
			map->redraw = 1;
			break;
		case SDL_SCANCODE_J:
		case SDL_SCANCODE_K:
			Camera_AdjustZStep(cam, code == SDL_SCANCODE_J ? -1.0f : 1.0f);
			map->redraw = 1;
			break;
		case SDL_SCANCODE_G:
			isGravitationEnabled = !isGravitationEnabled;
			velocity = 0.0f;
			break;
		case SDL_SCANCODE_O:
			map->optdist = 3000.0f;
			map->optimize ^= 1;
			map->redraw = 1;
			break;
		case SDL_SCANCODE_RETURN:
			if((mod & KMOD_ALT) != 0)
				Engine_ToggleFullscreen();
			break;
		case SDL_SCANCODE_ESCAPE:
			if(persistGrab) {
				persistGrab = 0;
				isMouseGrabbed = 1;
				ToggleMouseGrab();
				break;
			}

			Engine_Stop();
			break;
		default: break;
	}
}

static void ProcessKeyUp(SDL_Scancode code) {
	switch(code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_S:
			if(input[0] == code) input[0] = 0;
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_D:
			if(input[1] == code) input[1] = 0;
			break;
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_Q:
			if(input[2] == code) input[2] = 0;
			break;
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_F:
			if(input[3] == code) input[3] = 0;
			break;
		default: break;
	}
}

static int ProcessKeyboard(Camera *cam, float dm) {
	if(input[3])
		Camera_Pitch(cam, (input[3] == SDL_SCANCODE_R ? -dm : dm));
	if(input[2]) {
		if(isGravitationEnabled) {
			if(isGravitationEnabled && isOnTheGround) {
				velocity += INPUT_JUMP_VELOCITY;
				isOnTheGround = 0;
			}
		} else
			Camera_StrafeVert(cam, (input[2] == SDL_SCANCODE_Q ? -dm : dm));
	}

	if(input[1]) {
		float direction = (input[1] == SDL_SCANCODE_A ? -dm : dm);
		if(isMouseGrabbed)
			Camera_StrafeHoriz(cam, direction);
		else
			Camera_Rotate(cam, direction);
	}
	if(input[0]) {
		float speedMod = isGravitationEnabled ? 2.5 : 1.0;
		Camera_MoveForward(cam, (input[0] == SDL_SCANCODE_W ? -dm : dm) / speedMod, isGravitationEnabled);
	}

	/*
		Рисуем мир заново, если была нажата
		хоть какая-то кнопка из зарезервированных.
	*/
	for(int i = 0; i < INPUT_MAX_KEYBINDS; i++)
		if(input[i]) return 1;

	return 0;
}

static void ProcessMouseMotion(SDL_MouseMotionEvent *motion) {
	Map *map = NULL;
	Camera *cam = NULL;
	Engine_GetObjects(&cam, &map);
	cam->angle -= (float)motion->xrel * INPUT_MOUSE_SENS;
	cam->horizon -= (float)motion->yrel;
	map->redraw = 1;
}

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
				ProcessMouseMotion(&ev->motion);
			break;
	}
}
