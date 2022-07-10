#pragma once
#include "Graphics/GraphicsDevice.h"

class VertexBuffer
{
public:
	VertexBuffer() {};
	~VertexBuffer();
	void Bind();
	void Create(const void* data, uint32_t size, uint32_t sizeOfVertex, bool dynamic = false);
	void SetData(const void* data, uint32_t size, uint32_t sizeOfVertex);
	ID3D11Buffer* get_buffer_ptr() { return  vertex_buffer_ptr; };
private:
	ID3D11Buffer* vertex_buffer_ptr = nullptr;
	UINT vertex_stride = 0;
	UINT vertex_offset = 0;
	bool dynamic = false;
};

class IndexBuffer
{
public:
	//Data must be of type unsigned int
	IndexBuffer() {};
	~IndexBuffer();
	void Bind();
	void Create(const void* data, uint32_t size);
	ID3D11Buffer* get_buffer_ptr() { return  index_buffer_ptr; };
private:
	ID3D11Buffer* index_buffer_ptr = nullptr;
};

typedef enum CONST_BUFFER_USAGE
{
	CONST_BUFFER_USAGE_VERTEX = 0x1,
	CONST_BUFFER_USAGE_PIXEL = 0x2,
	CONST_BUFFER_USAGE_COMPUTE = 0x4,
} CONST_BUFFER_USAGE;

class ConstantBuffer
{
public:
	ConstantBuffer() {};
	//may only be used for the first time
	//Data must be sized in multiples of 16
	//bindShaderType -> CONST_BUFFER_USAGE_VERTEX
	//					CONST_BUFFER_USAGE_PIXEL
	//					CONST_BUFFER_USAGE_COMPUTE
	void Create(void* data, const int sizeOfData, int slot, CONST_BUFFER_USAGE bindShaderType);
	~ConstantBuffer();
	void SetData(void* data, const int sizeOfData);
	void Bind(int slot);
	ID3D11Buffer* get_buffer_ptr() { return  const_buffer_ptr; };
private:
	ID3D11Buffer* const_buffer_ptr = nullptr;
};