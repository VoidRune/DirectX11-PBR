#pragma once
#include "Graphics/GraphicsDevice.h"

class RenderTarget
{
public:
	RenderTarget() {};
	~RenderTarget();
	void Create(int width, int height);
	void BindAsTexture(int slot);
	void BindAsTarget();
	void Resize(int width, int height);
private:
	ID3D11ShaderResourceView* pTextureView;
	ID3D11RenderTargetView* pTargetView;
};

