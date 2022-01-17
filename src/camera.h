#ifndef VSCAMERA_H
#define VSCAMERA_H
#include <SDL_stdinc.h>
#include "defines.h"

typedef struct sCamera {
	Point position; // Текущая позиция камеры
	float height; // Высота камеры над картой
	float distance; // Дальность прорисовки
	float horizon; // Уровень горизонта
	float angle; // Угол поворота камеры
	float zstep; // Шаг по оси Z
} Camera;

static inline void Camera_AdjustDistance(Camera *cam, float value) {
	cam->distance += value;
	if(cam->distance < CAMERA_DISTANCE_MIN)
		cam->distance = CAMERA_DISTANCE_MAX;
	else if (cam->distance > CAMERA_DISTANCE_MAX)
		cam->distance = CAMERA_DISTANCE_MIN;
		
}

static inline void Camera_AdjustZStep(Camera *cam, float dir) {
	cam->zstep += dir * CAMERA_ZSTEP_MOD;
	cam->zstep = min(max(cam->zstep, CAMERA_ZSTEP_MIN), CAMERA_ZSTEP_MAX);
}

static inline void Camera_ResetDistance(Camera *cam) {
	cam->distance = CAMERA_DISTANCE_DEFAULT;
}

static inline void Camera_StrafeHoriz(Camera *cam, float spd) {
	cam->position.x +=  spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	cam->position.y +=  spd * CAMERA_MOVE_STEP * SDL_sinf(-cam->angle);
}

static inline void Camera_StrafeVert(Camera *cam, float spd) {
	cam->height += spd * CAMERA_MOVE_STEP;
}

static inline void Camera_Pitch(Camera *cam, float spd) {
	cam->horizon -= spd * CAMERA_HORIZON_STEP;
}

static inline void Camera_ResetPitch(Camera *cam) {
	cam->horizon = CAMERA_HORIZON_DEFAULT;
}

static inline void Camera_Rotate(Camera *cam, float spd) {
	cam->angle -= spd * CAMERA_ANGLE_STEP;
}

static inline void Camera_MoveForward(Camera *cam, float spd, int modHeight) {
	cam->position.x += spd * CAMERA_MOVE_STEP * SDL_sinf(cam->angle);
	cam->position.y += spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle);
	if(!modHeight)
		cam->height -= spd * (cam->horizon - CAMERA_HORIZON_DEFAULT) * CAMERA_HEIGHT_MOD;
}
#endif
