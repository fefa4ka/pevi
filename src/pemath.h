#pragma once
#include <raylib.h>
#include <raymath.h>

Vector2 CalculateBillboardAngles(Vector3 objectPosition, Vector3 cameraPosition,
                                 Vector3 cameraUp);

Color GenerateRandomColor(float s, float v);

Vector3 CalculateCameraPositionFromBillboard(Vector3 playerPosition,
                                             Vector2 billboardAngles,
                                             Vector3 cameraUp, float distance);
