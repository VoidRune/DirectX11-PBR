#pragma once
#include "Graphics/Buffers.h"

class ScreenQuad
{
public:
	ScreenQuad();
	~ScreenQuad();
	void Draw();
private:
	VertexBuffer vertexBuffer;
};

class Skybox
{
public:
	Skybox();
	~Skybox();
	void Draw();
private:
	VertexBuffer vertexBuffer;
};