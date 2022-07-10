#include "Buffers.h"

/* VERTEX BUFFER */


void VertexBuffer::Create(const void* data, uint32_t size, uint32_t sizeOfVertex, bool dynamic)
{
    vertex_stride = sizeOfVertex;
    vertex_offset = 0;

    LOG_IF_CORE_WARN(size == 0 && dynamic == false, "Size of data for vertex buffer is 0");

    D3D11_BUFFER_DESC vertex_buff_descr = {};
    vertex_buff_descr.ByteWidth = sizeOfVertex * size;
    vertex_buff_descr.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buff_descr.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    D3D11_SUBRESOURCE_DATA sr_data = { 0 };
    sr_data.pSysMem = data;

    HRESULT hr = GraphicsDevice::Get().device->CreateBuffer(
        &vertex_buff_descr,
        (data) ? &sr_data : nullptr,
        &vertex_buffer_ptr);
    assert(SUCCEEDED(hr));
}
void VertexBuffer::SetData(const void* data, uint32_t size, uint32_t sizeOfVertex)
{
    LOG_IF_CORE_WARN(dynamic == false, "Vertex buffer is not dynamic!");

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    GraphicsDevice::Get().context->Map(vertex_buffer_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, data, sizeOfVertex * size);
    GraphicsDevice::Get().context->Unmap(vertex_buffer_ptr, 0);
}
void VertexBuffer::Bind()
{
    if (!vertex_buffer_ptr)
    {
        LOG_CORE_WARN("vertex buffer is nullptr");
        return;
    }

    GraphicsDevice::Get().context->IASetVertexBuffers(
        0,
        1,
        &vertex_buffer_ptr,
        &vertex_stride,
        &vertex_offset);
}
VertexBuffer::~VertexBuffer()
{
    SafeRelease(&vertex_buffer_ptr);
}

/* INDEX BUFFER */
void IndexBuffer::Create(const void* data, uint32_t size)
{
    if (index_buffer_ptr != nullptr)
    {
        LOG_CORE_WARN("index buffer already loaded");
        return;
    }
    else if (size == 0)
    {
        LOG_CORE_WARN("Size of data for index buffer is 0");
        return;
    }
    D3D11_BUFFER_DESC index_buff_descr = {};
    index_buff_descr.ByteWidth = sizeof(uint32_t) * size;
    index_buff_descr.Usage = D3D11_USAGE_DEFAULT;
    index_buff_descr.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_buff_descr.StructureByteStride = sizeof(uint32_t);
    index_buff_descr.CPUAccessFlags = 0;
    index_buff_descr.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sr_data = { 0 };
    sr_data.pSysMem = data;

    HRESULT hr = GraphicsDevice::Get().device->CreateBuffer(
        &index_buff_descr,
        &sr_data,
        &index_buffer_ptr);
    assert(SUCCEEDED(hr));
}
IndexBuffer::~IndexBuffer()
{
    SafeRelease(&index_buffer_ptr);
}
void IndexBuffer::Bind()
{
    if (!index_buffer_ptr)
    {
        LOG_CORE_WARN("index buffer is nullptr");
        return;
    }

    GraphicsDevice::Get().context->IASetIndexBuffer(
        index_buffer_ptr,
        DXGI_FORMAT_R32_UINT,
        0);
}

/* CONSTANT BUFFER */
void ConstantBuffer::Create(void* data, const int sizeOfData, int slot, CONST_BUFFER_USAGE bindShaderType)
{
    if (const_buffer_ptr)
        return;

    D3D11_BUFFER_DESC const_buff_descr = {};
    const_buff_descr.ByteWidth = sizeOfData;
    const_buff_descr.Usage = D3D11_USAGE_DYNAMIC;
    const_buff_descr.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    const_buff_descr.StructureByteStride = 0;
    const_buff_descr.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    const_buff_descr.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sr_data = { 0 };
    sr_data.pSysMem = data;
    sr_data.SysMemPitch = 0;
    sr_data.SysMemSlicePitch = 0;

    HRESULT hr = GraphicsDevice::Get().device->CreateBuffer(
        &const_buff_descr,
        (data != nullptr) ? &sr_data : nullptr,
        &const_buffer_ptr);
    assert(SUCCEEDED(hr));

    if ((bindShaderType & CONST_BUFFER_USAGE_VERTEX) == 1)
        GraphicsDevice::Get().context->VSSetConstantBuffers(slot, 1, &const_buffer_ptr);
    if ((bindShaderType & CONST_BUFFER_USAGE_PIXEL) == 1)
        GraphicsDevice::Get().context->PSSetConstantBuffers(slot, 1, &const_buffer_ptr);
    if ((bindShaderType & CONST_BUFFER_USAGE_COMPUTE) == 1)
        GraphicsDevice::Get().context->CSSetConstantBuffers(slot, 1, &const_buffer_ptr);
}
void ConstantBuffer::SetData(void* data, const int sizeOfData)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    GraphicsDevice::Get().context->Map(const_buffer_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, data, sizeOfData);
    GraphicsDevice::Get().context->Unmap(const_buffer_ptr, 0);
}
void ConstantBuffer::Bind(int slot)
{
    GraphicsDevice::Get().context->VSSetConstantBuffers(slot, 1, &const_buffer_ptr);
}
ConstantBuffer::~ConstantBuffer()
{
    SafeRelease(&const_buffer_ptr);
}