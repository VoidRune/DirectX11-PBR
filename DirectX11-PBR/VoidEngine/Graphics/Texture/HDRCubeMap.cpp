#include "HDRCubeMap.h"
#include "Dependencies/stb/stb_image.h"
#include "Graphics/Geometry.h"
#include "Graphics/Shader.h"
#include <vector>
#include <thread>

/* HDR Texture */
void TextureHDR::loadFromFile(std::string name, bool linearFilter, bool flipTex)
{
    int nrChannels;
    stbi_set_flip_vertically_on_load(flipTex);
    stbi_info(name.c_str(), &m_Width, &m_Height, &nrChannels);
    float* data = stbi_loadf(name.c_str(), &m_Width, &m_Height, &nrChannels, (nrChannels == 3) ? 4 : 0);
    loadFromData(data, m_Width, m_Height, (nrChannels == 3) ? 4 : nrChannels, linearFilter);
}

void TextureHDR::loadFromData(void* data, int w, int h, int channels, bool linearFilter)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;         //0 = all mip levels
    desc.ArraySize = 1;
    desc.Format = (channels == 1) ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = data;
    sd.SysMemPitch = w * ((channels == 1) ? 1 : 4) * sizeof(float); // *4 because there are 4 values-> RGBA all 8bit
    sd.SysMemSlicePitch = w * h * ((channels == 1) ? 1 : 4) * sizeof(float);

    pTexture = {};
    HRESULT hr = GraphicsDevice::Get().device->CreateTexture2D(&desc, &sd, &pTexture);
    assert(SUCCEEDED(hr));

    stbi_image_free(data);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;           //-1 = all the mipmap levels

    hr = GraphicsDevice::Get().device->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);
    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = linearFilter ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    SamplerDesc.MipLODBias = 0.0f;
    SamplerDesc.MinLOD = 0.0f;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


    hr = GraphicsDevice::Get().device->CreateSamplerState(&SamplerDesc, &pSampler);
    assert(SUCCEEDED(hr));
}

TextureHDR::~TextureHDR()
{
    SafeRelease(&pTexture);
    SafeRelease(&pTextureView);
    SafeRelease(&pSampler);
}

void TextureHDR::Bind(unsigned int slot)
{
    GraphicsDevice::Get().context->PSSetSamplers(slot, 1, &pSampler);
    GraphicsDevice::Get().context->PSSetShaderResources(slot, 1, &pTextureView);
}

void CubeMapHDR::loadFromFile(std::string name, bool linearFilter, bool flipTex)
{

    TextureHDR t;
    t.loadFromFile(name, true, flipTex);

    XMMATRIX captureProjection = XMMatrixPerspectiveFovLH((XM_PI / 180) * 90, 1, 0.1, 10);
    XMMATRIX captureViews[] =
    {
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(1, 0, 0, 0), XMVectorSet(0, -1, 0, 0)),
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, -1, 0, 0)),
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(0,-1, 0, 0), XMVectorSet(0,  0, 1, 0)),
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0,  0,-1, 0)),
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 0,-1, 0), XMVectorSet(0, -1, 0, 0)),
        XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, -1, 0, 0))
    };

    //Create the TextureCube
    D3D11_TEXTURE2D_DESC textureDesc = {};

    const int SkyboxSize = 2048;
    const int IrradianceSize = 32;
    const int PreFilterSize = 256;

    textureDesc.Width = SkyboxSize;
    textureDesc.Height = SkyboxSize;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    ID3D11Texture2D* tex = nullptr;
    GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &tex);
    textureDesc.Width = IrradianceSize;
    textureDesc.Height = IrradianceSize;
    ID3D11Texture2D* texIrradiance = nullptr;
    GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &texIrradiance);
    textureDesc.Width = PreFilterSize;
    textureDesc.Height = PreFilterSize;
    ID3D11Texture2D* texPreFilter = nullptr;
    GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &texPreFilter);
    //textureDesc.Width = BrdfLookupSize;
    //textureDesc.Height = BrdfLookupSize;
    //ID3D11Texture2D* BrdfLookupSize = nullptr;
    //GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &BrdfLookupSize);

    // Create the Shader Resource view for the texture cube
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    GraphicsDevice::Get().device->CreateShaderResourceView(tex, &srvDesc, &pCubemapView);
    GraphicsDevice::Get().device->CreateShaderResourceView(texIrradiance, &srvDesc, &pIrradiaceMapView);
    GraphicsDevice::Get().device->CreateShaderResourceView(texPreFilter, &srvDesc, &pFilteredMapView);

    // Convolution texture
    //GraphicsDevice::Get().device->CreateTexture2D(&textureDesc, nullptr, &texConv);
    //GraphicsDevice::Get().device->CreateShaderResourceView(texConv, &srvDesc, &pTextureViewConvoluted);


    //Create the Render target views
    std::vector<ID3D11RenderTargetView*> rtvs;
    std::vector<ID3D11RenderTargetView*> preFilteredRtv;
    std::vector<ID3D11RenderTargetView*> IrradianceRtv;

    //std::vector<ID3D11RenderTargetView*> rtvsCon;

    for (uint32_t i = 0; i < 6; i++)
    {
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
        renderTargetViewDesc.Format = textureDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        renderTargetViewDesc.Texture2DArray.MipSlice = 0;
        renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
        renderTargetViewDesc.Texture2DArray.ArraySize = 1;

        ID3D11RenderTargetView* view1 = nullptr;
        ID3D11RenderTargetView* view2 = nullptr;
        ID3D11RenderTargetView* view3 = nullptr;

        //ID3D11RenderTargetView* viewConv = nullptr;
        GraphicsDevice::Get().device->CreateRenderTargetView(tex, &renderTargetViewDesc, &view1);
        GraphicsDevice::Get().device->CreateRenderTargetView(texIrradiance, &renderTargetViewDesc, &view2);
        GraphicsDevice::Get().device->CreateRenderTargetView(texPreFilter, &renderTargetViewDesc, &view3);

        //GraphicsDevice::Get().device->CreateRenderTargetView(texConv, &renderTargetViewDesc, &viewConv);
        rtvs.push_back(view1);
        IrradianceRtv.push_back(view2);
        preFilteredRtv.push_back(view3);

        //rtvsCon.push_back(viewConv);
    }
    tex->Release();
    texIrradiance->Release();
    texPreFilter->Release();


    //ENVIRONMENT MAP
    Skybox cmap;
    Shader s;
    s.Create("res/Shaders/EquirectangulatToCubemap.hlsl");

    ConstantBuffer cb;
    cb.Create(nullptr, 16 * sizeof(float), 0, CONST_BUFFER_USAGE_VERTEX);

    D3D11_VIEWPORT vp;
    vp.Width = SkyboxSize;
    vp.Height = SkyboxSize;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    GraphicsDevice::Get().context->RSSetViewports(1, &vp);

    s.Bind();
    t.Bind(0);
    for (uint32_t i = 0; i < 6; ++i)
    {
        float clearColor[4] = { 1.0f, 0.1f, 0.1f, 1.0f };
        GraphicsDevice::Get().context->ClearRenderTargetView(rtvs[i], clearColor);
        XMMATRIX d = XMMatrixTranspose(captureViews[i] * captureProjection);
        cb.SetData(&d, 16 * sizeof(float));
        cb.Bind(0);
        GraphicsDevice::Get().context->OMSetRenderTargets(1, &rtvs[i], nullptr);
        cmap.Draw();
    }

    // IRADIANCE

    Shader sirr;
    sirr.Create("res/Shaders/irradiance.hlsl");
    sirr.Bind();
    vp.Width = IrradianceSize;
    vp.Height = IrradianceSize;
    GraphicsDevice::Get().context->RSSetViewports(1, &vp);

    t.Bind(0);

    for (uint32_t i = 0; i < 6; ++i)
    {
        float clearColor[4] = { 1.0f, 0.1f, 0.1f, 1.0f };
        GraphicsDevice::Get().context->ClearRenderTargetView(IrradianceRtv[i], clearColor);
        XMMATRIX d = XMMatrixTranspose(captureViews[i] * captureProjection);
        cb.SetData(&d, 16 * sizeof(float));
        cb.Bind(0);
        GraphicsDevice::Get().context->OMSetRenderTargets(1, &IrradianceRtv[i], nullptr);
        cmap.Draw();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    //PREFILTERED MAP

    Shader spre;
    spre.Create("res/Shaders/preFilter.hlsl");
    spre.Bind();
    vp.Width = PreFilterSize;
    vp.Height = PreFilterSize;
    GraphicsDevice::Get().context->RSSetViewports(1, &vp);

    t.Bind(0);

    for (uint32_t i = 0; i < 6; ++i)
    {
        float clearColor[4] = { 1.0f, 0.1f, 0.1f, 1.0f };
        GraphicsDevice::Get().context->ClearRenderTargetView(preFilteredRtv[i], clearColor);
        XMMATRIX d = XMMatrixTranspose(captureViews[i] * captureProjection);
        cb.SetData(&d, 16 * sizeof(float));
        cb.Bind(0);
        GraphicsDevice::Get().context->OMSetRenderTargets(1, &preFilteredRtv[i], nullptr);
        cmap.Draw();
    }


    //GraphicsDevice::Get().context->PSSetSamplers(0, 1, &pSampler);
    //GraphicsDevice::Get().context->PSSetShaderResources(0, 1, &pTextureView);
    //Shader sCon;
    //sCon.Create("res/Shaders/convolution.hlsl");
    //sCon.Bind();
    //for (uint32_t i = 0; i < 6; ++i)
    //{
    //    float clearColor[4] = { 1.0f, 0.1f, 0.1f, 1.0f };
    //    GraphicsDevice::Get().context->ClearRenderTargetView(rtvsCon[i], clearColor);
    //    XMMATRIX d = XMMatrixTranspose(captureViews[i] * captureProjection);
    //    cb.SetData(&d, 16 * sizeof(float));
    //    cb.Bind(0);
    //    GraphicsDevice::Get().context->OMSetRenderTargets(1, &rtvsCon[i], nullptr);
    //    cmap.Draw();
    //}
    vp.Width = Window::Get().GetDimensions().x;
    vp.Height = Window::Get().GetDimensions().y;
    GraphicsDevice::Get().context->RSSetViewports(1, &vp);
}

CubeMapHDR::~CubeMapHDR()
{
    SafeRelease(&pCubemap);
    SafeRelease(&pCubemapView);
    SafeRelease(&pFilteredMapView);
    SafeRelease(&pSampler);
}

void CubeMapHDR::Bind(unsigned int slot)
{
    GraphicsDevice::Get().context->PSSetSamplers(0, 1, &pSampler);
    GraphicsDevice::Get().context->PSSetShaderResources(0, 1, &pCubemapView);
    GraphicsDevice::Get().context->PSSetShaderResources(6, 1, &pIrradiaceMapView);
    GraphicsDevice::Get().context->PSSetShaderResources(7, 1, &pFilteredMapView);
}