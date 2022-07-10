#include "RenderTarget.h"


void RenderTarget::Create(int width, int height)
{
	ID3D11Texture2D* pTexture;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = GraphicsDevice::Get().antiAlias;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &pTexture);

	//Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	if (GraphicsDevice::Get().antiAlias == 1)
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	else
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GraphicsDevice::Get().device->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);

	//Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	if (GraphicsDevice::Get().antiAlias == 1)
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	else
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
	GraphicsDevice::Get().device->CreateRenderTargetView(pTexture, &rtvDesc, &pTargetView);

	SafeRelease(&pTexture);
}

RenderTarget::~RenderTarget()
{
	SafeRelease(&pTargetView);
	SafeRelease(&pTextureView);
}

void RenderTarget::BindAsTexture(int slot)
{
	GraphicsDevice::Get().context->PSSetShaderResources(slot, 1, &pTextureView);
}

void RenderTarget::BindAsTarget()
{
	float color[4] = { 0.2f, 0.0f, 0.8f, 1.0f };
	GraphicsDevice::Get().context->ClearRenderTargetView(pTargetView, color);
	GraphicsDevice::Get().context->OMSetRenderTargets(1, &pTargetView, GraphicsDevice::Get().depthStencilView);
}

void RenderTarget::Resize(int width, int height)
{
	SafeRelease(&pTargetView);
	SafeRelease(&pTextureView);

	ID3D11Texture2D* pTexture;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = GraphicsDevice::Get().antiAlias;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &pTexture);

	//Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	if (GraphicsDevice::Get().antiAlias == 1)
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	else
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GraphicsDevice::Get().device->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);

	//Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	if (GraphicsDevice::Get().antiAlias == 1)
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	else
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
	GraphicsDevice::Get().device->CreateRenderTargetView(pTexture, &rtvDesc, &pTargetView);

	SafeRelease(&pTexture);
}