#include "constants.h"
#include "engine.h"
#include "camera.h"

void Camera_AdjustDistance(Camera *cam, float value) {
	cam->distance += value;
	if(cam->distance < 300.0f || cam->distance > 3000.0f)
		cam->distance = 300.0f;
}

void Camera_StrafeHoriz(Camera *cam, float spd) {
	cam->position.x +=  spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	cam->position.y +=  spd * CAMERA_MOVE_STEP * SDL_sinf(-cam->angle);
}

void Camera_StrafeVert(Camera *cam, float spd) {
	cam->height += spd * CAMERA_HEIGHT_STEP;
}

void Camera_Pitch(Camera *cam, float spd) {
	cam->horizon -= spd * CAMERA_HORIZON_STEP;
}

void Camera_ResetPitch(Camera *cam) {
	cam->horizon = CAMERA_HORIZON_DEFAULT;
}

void Camera_Rotate(Camera *cam, float spd) {
	cam->angle -= spd * CAMERA_ANGLE_STEP;
}

void Camera_MoveForward(Camera *cam, float spd) {
	cam->position.x += spd * CAMERA_MOVE_STEP * SDL_sinf(cam->angle);
	cam->position.y += spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	cam->height -= spd * (cam->horizon - CAMERA_HORIZON_DEFAULT) * CAMERA_HEIGHT_MOD;
}
