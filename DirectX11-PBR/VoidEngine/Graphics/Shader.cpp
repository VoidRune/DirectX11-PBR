#include "Shader.h"
#include <fstream>

void Shader::Create(std::string path)
{
    CreateVertexShader(path);
    CreatePixelShader(path);

    GraphicsDevice::Get().context->IASetInputLayout(input_layout_ptr);

}

void Shader::Create(std::string vertexSPath, std::string pixelSPath)
{
    CreateVertexShader(vertexSPath);
    CreatePixelShader(pixelSPath);

    GraphicsDevice::Get().context->IASetInputLayout(input_layout_ptr);
}

Shader::~Shader()
{
    SafeRelease(&vertex_shader_ptr);
    SafeRelease(&pixel_shader_ptr);
    SafeRelease(&input_layout_ptr);
}

void Shader::Bind()
{
    GraphicsDevice::Get().context->VSSetShader(vertex_shader_ptr, NULL, 0);
    GraphicsDevice::Get().context->PSSetShader(pixel_shader_ptr, NULL, 0);
    GraphicsDevice::Get().context->IASetInputLayout(input_layout_ptr);
}

void Shader::CreateVertexShader(std::string path)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ID3DBlob* blob_ptr = NULL, * error_blob = NULL;

    std::wstring stemp = std::wstring(path.begin(), path.end());
    LPCWSTR p = stemp.c_str();
    // COMPILE VERTEX SHADER
    HRESULT hr = D3DCompileFromFile(
        p,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain",
        "vs_5_0",
        flags,
        0,
        &blob_ptr,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            LOG_CORE_ERROR("Failed creating vertex shader: %s", path.c_str());
            LOG_CORE_INFO("%s", (char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        else
            LOG_CORE_ERROR("vertex shader: %s does not exist", path.c_str());

        if (blob_ptr) { blob_ptr->Release(); }
        assert(false);     
    }

    hr = GraphicsDevice::Get().device->CreateVertexShader(
        blob_ptr->GetBufferPointer(),
        blob_ptr->GetBufferSize(),
        NULL,
        &vertex_shader_ptr);
    assert(SUCCEEDED(hr));

    hr = CreateInputLayout(blob_ptr, &input_layout_ptr);
    LOG_IF_CORE_ERROR(FAILED(hr), "Failed creating input layout: %s", path.c_str());

    SafeRelease(&blob_ptr);
    SafeRelease(&error_blob);
}

void Shader::CreatePixelShader(std::string path)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ID3DBlob* blob_ptr = NULL, * error_blob = NULL;

    std::wstring stemp = std::wstring(path.begin(), path.end());
    LPCWSTR p = stemp.c_str();
    // COMPILE VERTEX SHADER
    HRESULT hr = D3DCompileFromFile(
        p,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain",
        "ps_5_0",
        flags,
        0,
        &blob_ptr,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            LOG_CORE_ERROR("Failed creating pixel shader: %s", path.c_str());
            LOG_CORE_INFO("%s", (char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        else
            LOG_CORE_ERROR("pixel shader: %s does not exist", path.c_str());

        if (blob_ptr) { blob_ptr->Release(); }
        assert(false);
    }

    hr = GraphicsDevice::Get().device->CreatePixelShader(
        blob_ptr->GetBufferPointer(),
        blob_ptr->GetBufferSize(),
        NULL,
        &pixel_shader_ptr);
    assert(SUCCEEDED(hr));

    SafeRelease(&blob_ptr);
    SafeRelease(&error_blob);
}

HRESULT Shader::CreateInputLayout(ID3DBlob* pShaderBlob, ID3D11InputLayout** pInputLayout)
{
    // Reflect shader info
    ID3D11ShaderReflection* pVertexShaderReflection = NULL;
    if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
    {
        return S_FALSE;
    }

    // Get shader info
    D3D11_SHADER_DESC shaderDesc;
    pVertexShaderReflection->GetDesc(&shaderDesc);

    // Read input layout description from shader info
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
    for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
    {
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

        // fill out input element desc
        D3D11_INPUT_ELEMENT_DESC elementDesc;
        elementDesc.SemanticName = paramDesc.SemanticName;
        elementDesc.SemanticIndex = paramDesc.SemanticIndex;
        elementDesc.InputSlot = 0;
        elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = 0;

        // determine DXGI format
        if (paramDesc.Mask == 1)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (paramDesc.Mask <= 3)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (paramDesc.Mask <= 7)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (paramDesc.Mask <= 15)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        //save element desc
        inputLayoutDesc.push_back(elementDesc);
    }

    // Try to create Input Layout
    HRESULT hr = GraphicsDevice::Get().device->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

    //Free allocation shader reflection memory
    pVertexShaderReflection->Release();
    return hr;
}

/* COMPUTE SHADER */

void ComputeShader::Create(std::string path, uint32_t numInputElements, uint32_t inputElementSize, uint32_t numOutputElements, uint32_t outputElementSize)
{
    LoadShader(path);
    inputDataSize = numInputElements * inputElementSize;
    outputDataSize = numOutputElements * outputElementSize;
    if(inputDataSize != 0)
        LoadInput(numInputElements, inputElementSize);
    LoadOutput(numOutputElements, outputElementSize);
}

ComputeShader::~ComputeShader()
{
    SafeRelease(&compute_shader_ptr);
    SafeRelease(&mOutputBuffer);
    SafeRelease(&mOutputResultBuffer);
    SafeRelease(&mOutputUAV);
}

void ComputeShader::Dispatch(int thread_groupX, int thread_groupY, int thread_groupZ, void* inputData, void* outputData)
{
    if (inputData)
    {
        D3D11_MAPPED_SUBRESOURCE inputMappedResource;
        ZeroMemory(&inputMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

        GraphicsDevice::Get().context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &inputMappedResource);

        memcpy(inputMappedResource.pData, inputData, inputDataSize);

        GraphicsDevice::Get().context->Unmap(mInputBuffer, 0);
    }

    GraphicsDevice::Get().context->CSSetShader(compute_shader_ptr, NULL, 0);
    if (inputData)
        GraphicsDevice::Get().context->CSSetShaderResources(0, 1, &mInputView);
    GraphicsDevice::Get().context->CSSetUnorderedAccessViews(0, 1, &mOutputUAV, NULL);

    GraphicsDevice::Get().context->Dispatch(thread_groupX, thread_groupY, thread_groupZ);
    // Copy result
    GraphicsDevice::Get().context->CopyResource(mOutputResultBuffer, mOutputBuffer);


    // Update particle system data with output from Compute Shader
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = GraphicsDevice::Get().context->Map(mOutputResultBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
    assert(SUCCEEDED(hr));

    memcpy(outputData, mappedResource.pData, outputDataSize);

    GraphicsDevice::Get().context->Unmap(mOutputResultBuffer, 0);
}

void ComputeShader::LoadShader(std::string path)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ID3DBlob* cs_blob_ptr = NULL, * error_blob = NULL;
    std::wstring stemp = std::wstring(path.begin(), path.end());
    LPCWSTR p = stemp.c_str();
    // COMPILE COMPUTE SHADER
    HRESULT hr = D3DCompileFromFile(
        p,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "CSMain",
        "cs_5_0",
        flags,
        0,
        &cs_blob_ptr,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (cs_blob_ptr) { cs_blob_ptr->Release(); }
        assert(false);
    }

    hr = GraphicsDevice::Get().device->CreateComputeShader(
        cs_blob_ptr->GetBufferPointer(),
        cs_blob_ptr->GetBufferSize(),
        NULL,
        &compute_shader_ptr);
    assert(SUCCEEDED(hr));

    SafeRelease(&cs_blob_ptr);
    SafeRelease(&error_blob);
}

void ComputeShader::LoadInput(uint32_t numInputElements, uint32_t inputElementSize)
{
    //STRUCTUDER BUFFER INPUT

    D3D11_BUFFER_DESC constantDataDesc = {};
    constantDataDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantDataDesc.ByteWidth = numInputElements * inputElementSize;
    constantDataDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_SHADER_RESOURCE;
    constantDataDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantDataDesc.StructureByteStride = inputElementSize;
    constantDataDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    /*
    float* d = new float[64 * 64];
    memset(d, 0.5f, 64 * 64 * sizeof(float));
    d[0] = 0.5f;
    d[1] = 0.5f;
    d[2] = 0.5f;
    d[3] = 0.5f;
    d[4] = 0.5f;
    d[5] = 0.5f;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = d;
    sd.SysMemPitch = 64 * sizeof(float);
    */

    HRESULT hr = GraphicsDevice::Get().device->CreateBuffer(&constantDataDesc, nullptr, &mInputBuffer);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.Flags = 0;
    srvDesc.BufferEx.NumElements = numInputElements;

    hr = GraphicsDevice::Get().device->CreateShaderResourceView(mInputBuffer, &srvDesc, &mInputView);
    assert(SUCCEEDED(hr));

}

void ComputeShader::LoadOutput(uint32_t numOutputElements, uint32_t outputElementSize)
{
    D3D11_BUFFER_DESC outputDesc;
    ZeroMemory(&outputDesc, sizeof(outputDesc));
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth = numOutputElements * outputElementSize;
    outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.StructureByteStride = outputElementSize;
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HRESULT hr = (GraphicsDevice::Get().device->CreateBuffer(&outputDesc, 0, &mOutputBuffer));
    assert(SUCCEEDED(hr));

    outputDesc.Usage = D3D11_USAGE_STAGING;
    outputDesc.BindFlags = 0;
    outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;


    hr = (GraphicsDevice::Get().device->CreateBuffer(&outputDesc, 0, &mOutputResultBuffer));
    assert(SUCCEEDED(hr));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = numOutputElements;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

    hr = GraphicsDevice::Get().device->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutputUAV);
    assert(SUCCEEDED(hr));
}