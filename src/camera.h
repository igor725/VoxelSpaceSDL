#ifndef VSCAMERA_H
#define VSCAMERA_H
#include <SDL_stdinc.h>
#include "defines.h"

typedef struct sCamera {
	Point position; // Текущая позиция камеры
	float height; // Высота камеры над картой
	float distance; // Дальность прорисовки
	float maxhorizon; // Максимальный уровень горизонта
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
	cam->horizon -= spd * cam->maxhorizon * CAMERA_HORIZON_STEP;
	// Ограничиваем горизонт камеры размерностью окна
	cam->horizon = max(-cam->maxhorizon, min(cam->horizon, cam->maxhorizon));
}

static inline void Camera_ResetPitch(Camera *cam) {
	cam->horizon = cam->maxhorizon / 2.0f;
}

static inline void Camera_Rotate(Camera *cam, float spd) {
	cam->angle -= spd * CAMERA_ANGLE_STEP;
}

static inline void Camera_MoveForward(Camera *cam, float spd, int modHeight) {
    float low = cam->horizon / cam->maxhorizon * 2 - 1;
    float low_module = SDL_sqrtf(1 + low*low);
    cam->position.x += spd * CAMERA_MOVE_STEP * SDL_sinf(cam->angle) / low_module;
    cam->position.y += spd * CAMERA_MOVE_STEP * SDL_cosf(cam->angle) / low_module;
    if(!modHeight)
        cam->height -= spd * CAMERA_MOVE_STEP * low / low_module;
}
#endif
