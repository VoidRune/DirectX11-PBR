#include "Geometry.h"
#include <vector>

ScreenQuad::ScreenQuad()
{
	struct Vertex
	{
		float x, y;
		float u, v;
	};
	std::vector<Vertex> vertices;
	vertices.emplace_back( -1, -1, 0, 1 );
	vertices.emplace_back( -1,  1, 0, 0 );
	vertices.emplace_back(  1,  1, 1, 0 );
	vertices.emplace_back(  1,  1, 1, 0 );
	vertices.emplace_back(  1, -1, 1, 1 );
	vertices.emplace_back( -1, -1, 0, 1 );
	vertexBuffer.Create(vertices.data(), vertices.size(), sizeof(Vertex));
}

ScreenQuad::~ScreenQuad()
{

}

void ScreenQuad::Draw()
{
	vertexBuffer.Bind();
	GraphicsDevice::Get().context->Draw(6, 0);
}

Skybox::Skybox()
{
	struct Vertex
	{
		float x, y, z;
	};
    std::vector<Vertex> vertices;
    //Front
    vertices.emplace_back(-0.5, -0.5, 0.5);
    vertices.emplace_back(-0.5, 0.5, 0.5);
    vertices.emplace_back(0.5, 0.5, 0.5);
    vertices.emplace_back(0.5, 0.5, 0.5);
    vertices.emplace_back(0.5, -0.5, 0.5);
    vertices.emplace_back(-0.5, -0.5, 0.5);
    //Right
    vertices.emplace_back(0.5, -0.5, 0.5);
    vertices.emplace_back(0.5, 0.5, 0.5);
    vertices.emplace_back(0.5, 0.5, -0.5);
    vertices.emplace_back(0.5, 0.5, -0.5);
    vertices.emplace_back(0.5, -0.5, -0.5);
    vertices.emplace_back(0.5, -0.5, 0.5);
    //Back
    vertices.emplace_back(0.5, -0.5, -0.5);
    vertices.emplace_back(0.5, 0.5, -0.5);
    vertices.emplace_back(-0.5, 0.5, -0.5);
    vertices.emplace_back(-0.5, 0.5, -0.5);
    vertices.emplace_back(-0.5, -0.5, -0.5);
    vertices.emplace_back(0.5, -0.5, -0.5);
    //Left
    vertices.emplace_back(-0.5, -0.5, -0.5);
    vertices.emplace_back(-0.5, 0.5, -0.5);
    vertices.emplace_back(-0.5, 0.5, 0.5);
    vertices.emplace_back(-0.5, 0.5, 0.5);
    vertices.emplace_back(-0.5, -0.5, 0.5);
    vertices.emplace_back(-0.5, -0.5, -0.5);
    //Top
    vertices.emplace_back(-0.5, 0.5, 0.5);
    vertices.emplace_back(-0.5, 0.5, -0.5);
    vertices.emplace_back(0.5, 0.5, -0.5);
    vertices.emplace_back(0.5, 0.5, -0.5);
    vertices.emplace_back(0.5, 0.5, 0.5);
    vertices.emplace_back(-0.5, 0.5, 0.5);
    //Bottom
    vertices.emplace_back(-0.5, -0.5, -0.5);
    vertices.emplace_back(-0.5, -0.5, 0.5);
    vertices.emplace_back(0.5, -0.5, 0.5);
    vertices.emplace_back(0.5, -0.5, 0.5);
    vertices.emplace_back(0.5, -0.5, -0.5);
    vertices.emplace_back(-0.5, -0.5, -0.5);

	vertexBuffer.Create(vertices.data(), vertices.size(), sizeof(Vertex));
}

Skybox::~Skybox()
{

}

void Skybox::Draw()
{
	vertexBuffer.Bind();
	GraphicsDevice::Get().context->Draw(36, 0);
}