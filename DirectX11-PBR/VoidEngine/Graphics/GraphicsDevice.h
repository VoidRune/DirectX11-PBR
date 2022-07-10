#pragma once
#include "Window/Window.h"
#include <assert.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>


#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

using namespace DirectX;

class GraphicsDevice
{
public:

	static inline GraphicsDevice& Get() { return *s_Instance; }
	GraphicsDevice();
	void Init(bool vsync = true, int antiAlias = 1, bool disableAltEnter = false);
	~GraphicsDevice();

	void SwapBuffer();
	void ClearBuffer();
	void Resize();
	void SetFullscreen(bool set);
	void SetRenderTarget();
private:
	void CreateDeviceAndSwapchain();
	void CreateFrameBuffer();
	void CreateDepthStencil();
	void CreateRasterizerState();
	void CreateBlendState();

	void DisableAltEnter();

	Window* win;
	static GraphicsDevice* s_Instance;

	GraphicsDevice(const GraphicsDevice&) = delete;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;

public:
	int videoCardMemory;
	char videoCardDescription[128];
	bool vsync;
	int antiAlias;

	IDXGISwapChain* swapChain;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11RasterizerState* rasterizerState;
	ID3D11BlendState* blendState;
};

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}