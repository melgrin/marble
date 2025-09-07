#include <raylib.h>
#include <rcamera.h>

#define CAMERA_MOVE_SPEED 0.9f
#define CAMERA_ROTATION_SPEED 0.03f
#define CAMERA_MOUSE_MOVE_SENSITIVITY 0.003f

// This is a custom implementation of raylib's UpdateCamera function.  Unlike UpdateCamera, it has variable move speed, and fixes straight-down camera jitter.  It only supports first person and free.
void UpdateCamera_custom(Camera* camera, int camera_mode) {
    static const bool rotate_around_target = false;
    static const bool lock_view = true; // setting this to true prevents jittering when looking straight down while also pressing down
    static const bool rotate_up = false;

    float move_speed = CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_LEFT_SHIFT)) move_speed *= 10.0f;
    if (IsKeyDown(KEY_RIGHT_SHIFT)) move_speed /= 10.0f;

    if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -CAMERA_ROTATION_SPEED, lock_view, rotate_around_target, rotate_up);
    if (IsKeyDown(KEY_UP  )) CameraPitch(camera,  CAMERA_ROTATION_SPEED, lock_view, rotate_around_target, rotate_up);

    if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -CAMERA_ROTATION_SPEED, rotate_around_target);
    if (IsKeyDown(KEY_LEFT )) CameraYaw(camera,  CAMERA_ROTATION_SPEED, rotate_around_target);

    bool move_in_world_plane = (camera_mode == CAMERA_FIRST_PERSON);

    if (IsKeyDown(KEY_W)) CameraMoveForward(camera,  move_speed, move_speed);
    if (IsKeyDown(KEY_A)) CameraMoveRight  (camera, -move_speed, move_speed);
    if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -move_speed, move_speed);
    if (IsKeyDown(KEY_D)) CameraMoveRight  (camera,  move_speed, move_speed);

    if (!move_in_world_plane) {
        if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, move_speed);
        if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -move_speed);
    }

    Vector2 mouse_position_delta = GetMouseDelta();
    CameraYaw(camera, -mouse_position_delta.x * CAMERA_MOUSE_MOVE_SENSITIVITY, rotate_around_target);
    CameraPitch(camera, -mouse_position_delta.y * CAMERA_MOUSE_MOVE_SENSITIVITY, lock_view, rotate_around_target, rotate_up);
}
