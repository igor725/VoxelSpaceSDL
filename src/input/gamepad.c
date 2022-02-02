#include <SDL_gamecontroller.h>
#include <SDL_version.h>
#include "defines.h"
#include "engine.h"

extern int isGravitationEnabled, isOnTheGround;
extern float velocity;

static SDL_GameController *pads[INPUT_MAX_PADS] = {0};
static int padButtonState[SDL_CONTROLLER_BUTTON_MAX] = {0};

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
