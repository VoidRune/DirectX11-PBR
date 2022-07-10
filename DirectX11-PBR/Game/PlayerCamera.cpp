#include "PlayerCamera.h"

PlayerCamera::PlayerCamera()
{
    camera = new Camera3D;
    camera->Create(70.0f, 0.1f, 1000.0f);

    SetCursorPos(windowCenter.x, windowCenter.y);
    Window::Get().SetCursorVisibility(false);

    position = &camera->position;
    *position = XMVectorSet(0, 0, -3, 0);
}

PlayerCamera::~PlayerCamera()
{
    delete camera;
}

void PlayerCamera::Update(float deltaTime)
{
    if (Input::IsKeyPressed(27))
    {
        Window::Get().SetCursorVisibility(lockCursor);
        lockCursor = !lockCursor;
        SetCursorPos(windowCenter.x, windowCenter.y);
    }
    if (lockCursor)
    {
        GetCursorPos(&MousePos);
        camera->yaw += (windowCenter.x - MousePos.x) * sensitivity;
        camera->pitch += (windowCenter.y - MousePos.y) * sensitivity;
        SetCursorPos(windowCenter.x, windowCenter.y);
    }

    camera->yaw = fmod(camera->yaw, 360.0f);

    if (camera->pitch > 89.0f) { camera->pitch = 89.0f; }
    if (camera->pitch < -89.0f) { camera->pitch = -89.0f; }

    temp = XMVectorSetX(temp, cos(XMConvertToRadians(camera->yaw)) * cos(XMConvertToRadians(camera->pitch)));
    temp = XMVectorSetZ(temp, sin(XMConvertToRadians(camera->yaw)) * cos(XMConvertToRadians(camera->pitch)));
    temp = XMVectorSetY(temp, sin(XMConvertToRadians(camera->pitch)));
    camera->forward = XMVector3Normalize(temp);
    camera->right = XMVector3Normalize(XMVector3Cross(-camera->forward, camera->up));

    XMVECTOR front = XMVector3Normalize(XMVectorSet(XMVectorGetX(camera->forward), 0, XMVectorGetZ(camera->forward), 0));
    XMVECTOR direction = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (Input::IsKeyDown('W')) { direction += front; }
    if (Input::IsKeyDown('S')) { direction -= front; }
    if (Input::IsKeyDown('D')) { direction += camera->right; }
    if (Input::IsKeyDown('A')) { direction -= camera->right; }
    if (Input::IsKeyDown('E')) /* zoom */
    {
        camera->projection = XMMatrixPerspectiveFovLH((XM_PI / 180) * 15, Window::Get().GetDimensions().x / Window::Get().GetDimensions().y, camera->nearPlane, camera->farPlane);
    }
    else
    {
        camera->projection = XMMatrixPerspectiveFovLH((XM_PI / 180) * camera->fov, Window::Get().GetDimensions().x / Window::Get().GetDimensions().y, camera->nearPlane, camera->farPlane);
    }

    if (Input::IsKeyDown('F')) { movementSpeed = 64; }
    else { movementSpeed = 4; }

    direction = XMVectorSetY(direction, 0);
    if (Input::IsKeyDown(' ')) { direction = XMVectorSetY(direction, 1); }
    if (Input::IsKeyDown(16)) { direction = XMVectorSetY(direction, -1); }

    direction = XMVector3Normalize(direction);
    *position = XMVectorAdd(*position, direction * deltaTime * movementSpeed);

    camera->view = XMMatrixLookAtLH(*position, DirectX::XMVectorAdd(*position, camera->forward), camera->up);

    camera->UpdateProjection();
}

void PlayerCamera::SetPosition(float x, float y, float z)
{
    *position = XMVectorSet(x, y, z, 0.0f);
}

void PlayerCamera::SetPosition(vec3f pos)
{
    *position = XMVectorSet(pos.x, pos.y, pos.z, 0.0f);
}

XMVECTOR PlayerCamera::GetXMPosition()
{
    return *position;
}