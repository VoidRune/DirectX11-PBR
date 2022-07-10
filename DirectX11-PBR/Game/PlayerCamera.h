#pragma once
#include "Graphics/Camera/Camera3D.h"

class PlayerCamera
{
public:
	PlayerCamera();
	~PlayerCamera();

	void Update(float deltaTime);
	void SetPosition(float x, float y, float z);
	void SetPosition(vec3f pos);
	XMVECTOR GetPosition() { return *position; };

	Camera3D* GetCamera() { return camera; }

	XMVECTOR GetXMPosition();
private:
	Camera3D* camera;

	float sensitivity = 0.1f;
	float movementSpeed = 4.0f;

	XMVECTOR* position;
	XMVECTOR temp;
	//float eyeHeight = 0.85f;
	BoundingBox hitbox;

	vec2f windowCenter = { Window::Get().GetMonitorDimensions().x / 2, Window::Get().GetMonitorDimensions().y / 2 };
	POINT MousePos = {};

	bool lockCursor = true;
};