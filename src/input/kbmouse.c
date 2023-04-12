#include <SDL_scancode.h>
#include <SDL_keycode.h>

#include "../defines.h"
#include "../engine.h"
#include "../camera.h"

extern int isGravitationEnabled, isOnTheGround;
extern float velocity;

static SDL_Scancode input[INPUT_MAX_KEYBINDS] = {0};
static int isMouseGrabbed = 0, persistGrab = 0, isSprintActive = 0;

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
		case SDL_SCANCODE_LSHIFT:
			isSprintActive = 1;
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
		case SDL_SCANCODE_LSHIFT:
			isSprintActive = 0;
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
		float speedMod = isGravitationEnabled ? 2.5f : 1.0f;
		if (isSprintActive) speedMod *= 0.3f;
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

static void ProcessMouseMotion(Sint32 rx, Sint32 ry) {
	Map *map = NULL;
	Camera *cam = NULL;
	Engine_GetObjects(&cam, &map);
	cam->angle -= (float)rx * INPUT_MOUSE_SENS;
	cam->horizon -= (float)ry * cam->maxhorizon * INPUT_MOUSE_SENS;
	map->redraw = 1;
}
