#ifndef VSCAMERA_H
#define VSCAMERA_H
#include "types.h"

void Camera_AdjustDistance(Camera *cam, float value);
void Camera_StrafeHoriz(Camera *cam, float spd);
void Camera_StrafeVert(Camera *cam, float spd);
void Camera_Pitch(Camera *cam, float spd);
void Camera_ResetPitch(Camera *cam);
void Camera_Rotate(Camera *cam, float spd);
void Camera_MoveForward(Camera *cam, float spd, int modHeight);
#endif
