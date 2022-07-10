#include "Material.h"


Material::Material()
{

}

Material::~Material()
{

}

void Material::loadFromPath(std::string path)
{
	albedo.loadFromFile(path + "albedo.png", true);
	normal.loadFromFile(path + "normal.png", true);
	metallic.loadFromFile(path + "metallic.png", true);
	roughness.loadFromFile(path + "roughness.png", true);
}

void Material::Bind()
{
	albedo.Bind(1);
	normal.Bind(2);
	metallic.Bind(3);
	roughness.Bind(4);
}