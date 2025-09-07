#include "rlgl.h"
#include "raymath.h"

// [XXX nearly exact copy of raylib 5.0, but returns {-1,-1} if position is not on the screen.]
// Get size position for a 3d world space position (useful for texture drawing)
Vector2 _GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height)
{
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    // todo: Why not use Vector3Transform(Vector3 v, Matrix mat)?

    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // Transform world position to view
    worldPos = QuaternionTransform(worldPos, matView);

    // Transform result to projection (clip space position)
    worldPos = QuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // [XXX this is the one change to the original function.  this is the condition when the point is behind thew viewing plane.]
    if (fabs(ndcPos.z) > 1 || fabs(ndcPos.x) > 1 || fabs(ndcPos.y) > 1) {
        return (Vector2){-1,-1};
    }
    // [XXX]

    // Calculate 2d screen position vector
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };

    return screenPosition;
}

// [XXX exact copy of raylib 5.0, but using "_GetWorldToScreen"]
// Get the screen space position from a 3d world space position
Vector2 _GetWorldToScreen(Vector3 position, Camera camera)
{
    Vector2 screenPosition = _GetWorldToScreenEx(position, camera, GetScreenWidth(), GetScreenHeight());
    return screenPosition;
}

