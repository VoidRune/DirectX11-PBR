#pragma once
#include <vector>
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Buffers.h"
#pragma comment(lib, "dxguid.lib")


class Shader
{
public:
	Shader() {};
	/* Loads vertex & pixels shader from same file */
	void Create(std::string path);
	/* Loads vertex & pixels shader from separate files */
	void Create(std::string vertexSPath, std::string pixelSPath);
	~Shader();

	void Bind();
private:
	void CreateVertexShader(std::string path);
	void CreatePixelShader(std::string path);
	HRESULT CreateInputLayout(ID3DBlob* pShaderBlob, ID3D11InputLayout** pInputLayout);

	ID3D11VertexShader* vertex_shader_ptr = {};
	ID3D11PixelShader* pixel_shader_ptr = {};

	ID3D11InputLayout* input_layout_ptr = {};
};

class ComputeShader
{
public:
	ComputeShader() {};
	~ComputeShader();

	/* inputSize -> size of input data in BYTES */
	/* in numthreads(X, Y, Z) X*Y*Z cant be greater than 1024 */
	void Create(std::string path, uint32_t numInputElements, uint32_t inputElementSize, uint32_t numOutputElements, uint32_t outputElementSize);
	void Dispatch(int thread_groupX, int thread_groupY, int thread_groupZ, void* inputData, void* outputData);

private:
	void LoadShader(std::string path);
	void LoadInput(uint32_t numInputElements, uint32_t inputElementSize);
	void LoadOutput(uint32_t numOutputElements, uint32_t outputElementSize);

	ID3D11ComputeShader* compute_shader_ptr;
	//Input
	ID3D11Buffer* mInputBuffer;
	ID3D11ShaderResourceView* mInputView;
	uint32_t inputDataSize;
	//Output
	ID3D11Buffer* mOutputBuffer;
	ID3D11Buffer* mOutputResultBuffer;
	ID3D11UnorderedAccessView* mOutputUAV;
	uint32_t outputDataSize;

};