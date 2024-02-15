#include "pemath.h"

Vector2 CalculateBillboardAngles(Vector3 objectPosition, Vector3 cameraPosition,
                                 Vector3 cameraUp)
{
    Vector3 direction
        = Vector3Normalize(Vector3Subtract(cameraPosition, objectPosition));

    // Calculate yaw (horizontal rotation)
    float yaw = atan2f(direction.x, direction.z);

    // Calculate pitch (vertical rotation)
    // Adjust the direction with respect to camera's up vector
    Vector3 right = Vector3CrossProduct(cameraUp, direction);
    direction
        = Vector3CrossProduct(direction, right); // Re-orthogonalize direction

    float pitch = asinf(direction.y);

    return (Vector2){pitch, yaw};
}

// Generates a nice color with a random hue
Color GenerateRandomColor(float s, float v)
{
    const float Phi = 0.618033988749895f; // Golden ratio conjugate
    float       h   = (float)GetRandomValue(0, 360);
    h               = fmodf((h + h * Phi), 360.0f);
    return ColorFromHSV(h, s, v);
}

Vector3 CalculateCameraPositionFromBillboard(Vector3 playerPosition,
                                             Vector2 billboardAngles,
                                             Vector3 cameraUp, float distance)
{
    // Calculate the direction vector from player to billboard
    Vector3 direction;
    direction.x = sinf(billboardAngles.x) * cosf(billboardAngles.y);
    direction.y = sinf(billboardAngles.y);
    direction.z = cosf(billboardAngles.x) * cosf(billboardAngles.y);

    // Calculate the right vector based on camera up vector
    Vector3 right = Vector3CrossProduct(cameraUp, direction);

    // Calculate the camera position
    Vector3 cameraPosition;
    cameraPosition.x = playerPosition.x + direction.x * distance;
    cameraPosition.y = playerPosition.y + direction.y * distance;
    cameraPosition.z = playerPosition.z + direction.z * distance;

    return cameraPosition;
}
