#include "Texture.h"
#include "Dependencies/stb/stb_image.h"
//#include "Utility/stb_image_write.h"
#include <string>
#include <filesystem>
#include "Graphics/Geometry.h"
#include "Graphics/Shader.h"
#include "Graphics/Buffers.h"

void Texture::loadFromFile(std::string name, bool linearFilter, bool flipTex)
{
    int nrChannels;
    stbi_set_flip_vertically_on_load(flipTex);
    stbi_info(name.c_str(), &m_Width, &m_Height, &nrChannels);
    unsigned char* data = stbi_load(name.c_str(), &m_Width, &m_Height, &nrChannels, (nrChannels == 3) ? 4 : 0);
    loadFromData(data, m_Width, m_Height, (nrChannels == 3) ? 4 : nrChannels, linearFilter);
}

void Texture::loadFromData(void* data, int w, int h, int channels, bool linearFilter)
{
    if (data == nullptr)
    {
        LOG_CORE_ERROR("Failed to load texture from data");
        data = new unsigned char[16]
        {
            0, 0, 0, 255,
            230, 0, 230, 255,
            230, 0, 230, 255,
            0, 0, 0, 255
        };
        m_Width = 2;
        m_Height = 2;
        linearFilter = false;
    }
    else
    {
        m_Width = w;
        m_Height = h;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_Width;
    desc.Height = m_Height;
    desc.MipLevels = 1;         //0 = all mip levels
    desc.ArraySize = 1;
    desc.Format = (channels == 1) ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = data;
    sd.SysMemPitch = m_Width * ((channels == 1) ? 1 : 4) * sizeof(unsigned char); // *4 because there are 4 values-> RGBA all 8bit
    sd.SysMemSlicePitch = m_Width * m_Height * ((channels == 1) ? 1 : 4) * sizeof(unsigned char);

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

Texture::~Texture()
{
    SafeRelease(&pTexture);
    SafeRelease(&pTextureView);
    SafeRelease(&pSampler);
}

void Texture::Bind(unsigned int slot)
{
    GraphicsDevice::Get().context->PSSetSamplers(slot, 1, &pSampler);
    GraphicsDevice::Get().context->PSSetShaderResources(slot, 1, &pTextureView);
}
/* Texture array */

TextureArray::TextureArray(int arraySize) :
    arraySize(arraySize)
{
    sd = new D3D11_SUBRESOURCE_DATA[arraySize];
    data = new unsigned char*[arraySize] ;
}

void TextureArray::AddTexture(std::string name, bool flipTex)
{
    int nrChannels;
    stbi_set_flip_vertically_on_load(flipTex);
    data[textureIndex] = stbi_load(name.c_str(), &m_Width, &m_Height, &nrChannels, (std::filesystem::path(name).extension().string() == ".png") ? 0 : 4);
    if (data[textureIndex] == nullptr)
    {
        LOG_CORE_ERROR("Failed to load texture: %s", name.c_str());
        unsigned char* tempData = new unsigned char[m_Width * m_Height * 4];
        for (size_t i = 0; i < m_Width * m_Height * 4; i+=4)
        {
            tempData[i  ] = 230;
            tempData[i+1] = 0;
            tempData[i+2] = 230;
            tempData[i+3] = 255;
        }
        data[textureIndex] = tempData;
    }
    sd[textureIndex].pSysMem = data[textureIndex];
    sd[textureIndex].SysMemPitch = m_Width * 4 * sizeof(unsigned char);
    sd[textureIndex].SysMemSlicePitch = m_Width * m_Height * 4 * sizeof(unsigned char);
    textureIndex++;
}

void TextureArray::Finish(bool linearFilter)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_Width;
    desc.Height = m_Height;
    desc.MipLevels = 1;         //0 = all mip levels
    desc.ArraySize = arraySize;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    pTexture = {};
    HRESULT hr = GraphicsDevice::Get().device->CreateTexture2D(&desc, sd, &pTexture);
    assert(SUCCEEDED(hr));
    for (size_t i = 0; i < arraySize; i++)
    {
        stbi_image_free(data[i]);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;           //-1 = all the mipmap levels
    srvDesc.Texture2DArray.ArraySize = arraySize;
    srvDesc.Texture2DArray.MipLevels = 1;

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

    delete sd;
    hr = GraphicsDevice::Get().device->CreateSamplerState(&SamplerDesc, &pSampler);
    assert(SUCCEEDED(hr));
}

TextureArray::~TextureArray()
{
    SafeRelease(&pTexture);
    SafeRelease(&pTextureView);
    SafeRelease(&pSampler);
    if (data != nullptr) delete[] data;
}

void TextureArray::Bind(unsigned int slot)
{
    GraphicsDevice::Get().context->PSSetSamplers(slot, 1, &pSampler);
    GraphicsDevice::Get().context->PSSetShaderResources(slot, 1, &pTextureView);
}

/* Cubemap */
void Cubemap::loadFromPath(std::string path, bool linearFilter)
{

    int nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data[6];
    stbi_info((path + "front.jpg").c_str(), &m_Width, &m_Height, &nrChannels);
    const char* sides[6] = { "right","left","top","bottom","front","back" };
    for (int i = 0; i < 6; i++)
    {
        data[i] = stbi_load((path + sides[i] + ".jpg").c_str(), &m_Width, &m_Height, &nrChannels, (nrChannels == 3) ? 4 : 0);
    }
    if (nrChannels == 3) nrChannels = 4;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_Width;
    desc.Height = m_Height;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SUBRESOURCE_DATA sd[6];
    for (int i = 0; i < 6; i++)
    {
        sd[i].pSysMem = data[i];
        sd[i].SysMemPitch = m_Width * nrChannels * sizeof(unsigned char);
        sd[i].SysMemSlicePitch = m_Width * m_Height * nrChannels * sizeof(unsigned char);
    }

    HRESULT hr = GraphicsDevice::Get().device->CreateTexture2D(&desc, sd, &pTexture);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
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

void Cubemap::Bind(int slot)
{
    GraphicsDevice::Get().context->PSSetSamplers(slot, 1, &pSampler);
    GraphicsDevice::Get().context->PSSetShaderResources(slot, 1, &pTextureView);
}

Cubemap::~Cubemap()
{
    SafeRelease(&pTexture);
    SafeRelease(&pTextureView);
    SafeRelease(&pSampler);
}

/* Texture batch */

void TextureBatch::AddTexture(std::string path)
{
    int x, y, comp;
    stbi_info(path.c_str(), &x, &y, &comp);

    coordinates.emplace(path, vec4f{(float)incX, (float)incY, incX + (float)x, incY + (float)y});

    incX += x;
    if ( y > maxIncY) { maxIncY =  y; }
    if (incX > lineX) { lineX = incX; }
    if (incY > lineY) { lineY = incY; }
    while (lineX > m_Width || lineY > m_Height) 
    { m_Width *= 2; m_Height *= 2; }

    if (incX >= maxIncY * 2.5f) { incX = 0; incY += maxIncY; maxIncY = 0; }

}
void TextureBatch::Finish(bool linearFilter, bool flipTex)
{
    data = new Pixel[m_Width * m_Height];
    stbi_set_flip_vertically_on_load(flipTex);

    for (auto& c : coordinates)
    {
        int w;
        int h;
        int channels;
        unsigned char* rawData = stbi_load(c.first.c_str(), &w, &h, &channels, 4);
        for (int y = 0; y < h; y++)
        {
            memcpy(&data[(size_t)(c.second.x + (c.second.y + y) * m_Width)], &rawData[y * w * sizeof(Pixel)], w * sizeof(Pixel));
        }
        stbi_image_free(rawData);
        if (!linearFilter)
        {
            c.second.x = (c.second.x + 0.05f) / (float)m_Width;
            c.second.y = (c.second.y + 0.05f) / (float)m_Height;
            c.second.z = (c.second.z - 0.05f) / (float)m_Width;
            c.second.w = (c.second.w - 0.05f) / (float)m_Height;
        }
        else
        {
            c.second.x /= (float)m_Width;
            c.second.y /= (float)m_Height;
            c.second.z /= (float)m_Width;
            c.second.w /= (float)m_Height;
        }
    }

    //stbi_write_png("test.png", m_Width, m_Height, 4, data, m_Width * sizeof(Pixel));
    batchedTex.loadFromData(data, m_Width, m_Height, 4, linearFilter);
}
vec4f TextureBatch::get_coords(std::string key)
{
    return coordinates.at(key);
}
void TextureBatch::Bind(unsigned int slot)
{
    batchedTex.Bind(slot);
}

