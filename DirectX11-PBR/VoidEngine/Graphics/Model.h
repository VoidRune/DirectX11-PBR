#pragma once
#include "Graphics/Buffers.h"
#include <vector>

class Model
{
public:
	Model() {};
	~Model() {};
	void Create(std::string fileName);
	void Draw();

private:
	void LoadOBJ(std::string fileName);
	void LoadFBX(std::string fileName);

	VertexBuffer vertexBuffer;
	std::vector<IndexBuffer> indexBuffer;
	std::vector<uint32_t> index_count;

};