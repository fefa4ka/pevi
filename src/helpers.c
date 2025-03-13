#include "helpers.h"

bool object_is_hovered(Camera_t *camera, BoundingBox object, Plane *plane) {
  Vector3 size = Vector3Subtract(object.max, object.min);
  Vector3 center = Vector3Add(object.min, Vector3Scale(size, 0.5f));
  // Build transformation matrix
  Matrix transform = MatrixIdentity();
  // First translate to origin
  transform =
      MatrixMultiply(transform, MatrixTranslate(center.x, -center.y, center.z));
  // Apply rotations
  transform = MatrixMultiply(transform, MatrixRotateX(plane->angles.x));
  transform = MatrixMultiply(transform, MatrixRotateY(plane->angles.y));
  transform = MatrixMultiply(transform, MatrixRotateZ(plane->angles.z));
  // Translate to final position
  transform = MatrixMultiply(
      transform, MatrixTranslate(plane->pos.x, plane->pos.y, plane->pos.z));

  // Create transformed bounding box
  Vector3 min_position = {-size.x / 2, -size.y / 2, -size.z / 2};
  Vector3 max_position = {size.x / 2, size.y / 2, size.z / 2};
  BoundingBox box = (BoundingBox){Vector3Transform(min_position, transform),
                                  Vector3Transform(max_position, transform)};

  // Check collisions
  return GetRayCollisionBox(camera->ray, box).hit ||
         GetRayCollisionBox(camera->ray_center, box).hit;
}
