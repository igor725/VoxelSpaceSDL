#ifndef VSCAMERA_H
#define VSCAMERA_H
#include "SDL_stdinc.h"
#include "constants.h"
#include "types.h"

inline void Camera_AdjustDistance(Camera *cam, float value) {
	cam->distance += value;
	if(cam->distance < 300.0f || cam->distance > 3000.0f)
		cam->distance = 300.0f;
}

inline void Camera_StrafeHoriz(Camera *cam, float spd) {
	cam->position.x +=  spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	cam->position.y +=  spd * CAMERA_MOVE_STEP * SDL_sinf(-cam->angle);
}

inline void Camera_StrafeVert(Camera *cam, float spd) {
	cam->height += spd * CAMERA_HEIGHT_STEP;
}

inline void Camera_Pitch(Camera *cam, float spd) {
	cam->horizon -= spd * CAMERA_HORIZON_STEP;
}

inline void Camera_ResetPitch(Camera *cam) {
	cam->horizon = CAMERA_HORIZON_DEFAULT;
}

inline void Camera_Rotate(Camera *cam, float spd) {
	cam->angle -= spd * CAMERA_ANGLE_STEP;
}

inline void Camera_MoveForward(Camera *cam, float spd, int modHeight) {
	cam->position.x += spd * CAMERA_MOVE_STEP * SDL_sinf(cam->angle);
	cam->position.y += spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	if(!modHeight)
		cam->height -= spd * (cam->horizon - CAMERA_HORIZON_DEFAULT) * CAMERA_HEIGHT_MOD;
}
#endif
