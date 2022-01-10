#include "constants.h"
#include "engine.h"
#include "camera.h"
#include "input.h"

static SDL_GameController *controllers[INPUT_MAX_CONTROLLERS] = {0};
static int controllerButtonsState[SDL_CONTROLLER_BUTTON_MAX] = {0};
static SDL_Scancode input[INPUT_MAX_KEYBINDS] = {0};
static int isMouseGrabbed = 0;

static void AddController(Sint32 id) {
	if(id < INPUT_MAX_CONTROLLERS && SDL_IsGameController(id)) {
		SDL_GameController *controller = SDL_GameControllerOpen(id);
		if(controller) {
			controllers[id] = controller;
			SDL_Log("Found controller: %s", SDL_GameControllerName(controller));
		} else SDL_Log("Failed to open controller #%d: %s", id, SDL_GetError());
	}
}

static void RemoveController(Sint32 id) {
	if(id < INPUT_MAX_CONTROLLERS && controllers[id]) {
		SDL_Log("Controller %d disconnected", id);
		SDL_GameControllerClose(controllers[id]);
	}
}

static int ProcessControllerButtonDown(Camera *cam, SDL_GameControllerButton btn) {
	switch(btn) {
		case SDL_CONTROLLER_BUTTON_B:
			Engine_Stop();
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			Camera_AdjustDistance(cam, CAMERA_DISTANCE_DEFAULT - cam->distance);
			return 1;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			Camera_AdjustDistance(cam, btn == SDL_CONTROLLER_BUTTON_LEFTSHOULDER ? -CAMERA_DISTANCE_STEP : CAMERA_DISTANCE_STEP);
			return 1;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			Camera_ResetPitch(cam);
			return 1;
		default: break;
	}

	return 0;
}

static int ProcessControllerButtonHold(Camera *cam, SDL_GameControllerButton btn, float dm) {
	switch (btn) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			Camera_StrafeVert(cam, (btn == SDL_CONTROLLER_BUTTON_DPAD_UP ? dm : -dm));
			return 1;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			Camera_StrafeHoriz(cam, (btn == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? -dm : dm));
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
		Camera_MoveForward(cam, leftStick.y * speedup * dm);
		Camera_StrafeHoriz(cam, leftStick.x * speedup * dm);
		handled = 1;
	}

	if(SDL_fabsf(rightStick.x) > 0.15f || SDL_fabsf(rightStick.y) > 0.15f) {
		float speedup = 1.0f + trigger.y;
		Camera_Rotate(cam, rightStick.x * speedup * dm);
		Camera_Pitch(cam, rightStick.y * speedup * dm);
		handled = 1;
	}

	for(SDL_GameControllerButton i = 1; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
		if(SDL_GameControllerGetButton(pad, i)) {
			if(!controllerButtonsState[i]) {
				if(ProcessControllerButtonDown(cam, i)) handled = 1;
				controllerButtonsState[i] = 1;
			} else if(ProcessControllerButtonHold(cam, i, dm)) handled = 1;
		} else {
			if(controllerButtonsState[i]) {
				// ProcessControllerButtonUp(i);
				controllerButtonsState[i] = 0;
			}
		}
	}

	return handled;
}

static int PollControllers(Camera *cam, float dm) {
	for(Sint32 id = 0; id < INPUT_MAX_CONTROLLERS; id++) {
		SDL_GameController *pad = controllers[id];
		if(pad && PollController(pad, cam, dm)) return 1;
	}

	return 0;
}

#ifndef min
#define min(a, b) (((a)<(b))?(a):(b))
#define max(a, b) (((a)>(b))?(a):(b))
#endif

static void ProcessKeyDown(SDL_KeyboardEvent *ev) {
	Camera *cam = NULL;
	Map *map = NULL;
	Engine_GetObjects(&cam, &map);
	SDL_Scancode code = ev->keysym.scancode;
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
			if(ev->keysym.mod & KMOD_CTRL)
				Camera_AdjustDistance(cam, CAMERA_DISTANCE_DEFAULT - cam->distance);
			else
				Camera_AdjustDistance(cam, ev->keysym.mod & KMOD_SHIFT ? -CAMERA_DISTANCE_STEP : CAMERA_DISTANCE_STEP);
			break;
		case SDL_SCANCODE_C:
			Camera_ResetPitch(cam);
			map->redraw = 1;
			break;
		case SDL_SCANCODE_J:
			cam->zstep = max(CAMERA_ZSTEP_MIN, (cam->zstep -= CAMERA_ZSTEP_STEP));
			map->redraw = 1;
			break;
		case SDL_SCANCODE_K:
			cam->zstep = min(CAMERA_ZSTEP_MAX, (cam->zstep += CAMERA_ZSTEP_STEP));
			map->redraw = 1;
			break;
		case SDL_SCANCODE_ESCAPE:
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
	if(input[2])
		Camera_StrafeVert(cam, (input[2] == SDL_SCANCODE_Q ? -dm : dm));
	if(input[1]) {
		float direction = (input[1] == SDL_SCANCODE_A ? -dm : dm);
		if(isMouseGrabbed)
			Camera_StrafeHoriz(cam, direction);
		else
			Camera_Rotate(cam, direction);
	}
	if(input[0])
		Camera_MoveForward(cam, (input[0] == SDL_SCANCODE_W ? -dm : dm));

	/*
		Рисуем мир заново, если была нажата
		хоть какая-то кнопка из зарезервированных.
	*/
	for(int i = 0; i < INPUT_MAX_KEYBINDS; i++)
		if(input[i]) return 1;
	
	return 0;
}

static void ToggleMouseGrab(void) {
	isMouseGrabbed = !isMouseGrabbed;
	SDL_SetRelativeMouseMode(isMouseGrabbed);
	SDL_SetWindowGrab(Engine_GetWindow(), isMouseGrabbed);
}

static void ProcessMouseMotion(SDL_MouseMotionEvent *motion) {
	Map *map = NULL;
	Camera *cam = NULL;
	Engine_GetObjects(&cam, &map);
	cam->angle -= (float)motion->xrel * (MOUSE_SENS / (float)Engine_GetDeltaTime());
	cam->horizon -= (float)motion->yrel;
	map->redraw = 1;
}

void Input_Update(void *ptr) {
	float deltaMult = ((float)*(int *)ptr) * 0.03f;
	Map *map = NULL;
	Camera *cam = NULL;
	Engine_GetObjects(&cam, &map);

	if(PollControllers(cam, deltaMult) || ProcessKeyboard(cam, deltaMult)) {
		// Вытаскиваем камеру из-под земли, если она там и перерисовываем мир
		float minHeight = (float)Map_GetHeight(map, &cam->position) + 10.0f;
		cam->height = max(minHeight, min(cam->height, 1000.0f));
		cam->horizon = max(-WINDOW_HEIGHT, min(cam->horizon, WINDOW_HEIGHT));
		if(SDL_fabsf(cam->angle) > M_PI * 2) cam->angle = 0;
		map->redraw = 1;
	}
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
			ProcessKeyDown(&ev->key);
			break;
		case SDL_KEYUP:
			ProcessKeyUp(ev->key.keysym.scancode);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if(ev->button.button == SDL_BUTTON_LEFT)
				ToggleMouseGrab();
			break;
		case SDL_MOUSEMOTION:
			if(isMouseGrabbed)
				ProcessMouseMotion(&ev->motion);
			break;
	}
}