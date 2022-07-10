#pragma once
#include "Graphics/Texture/Texture.h"

class Material
{
public:
	Material();
	~Material();

	void loadFromPath(std::string path);
	void Bind();

private:
	Texture albedo;
	Texture normal;
	Texture metallic;
	Texture roughness;
};