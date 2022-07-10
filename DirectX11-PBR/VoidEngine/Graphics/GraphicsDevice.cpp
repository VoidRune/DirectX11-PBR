#include "GraphicsDevice.h"

GraphicsDevice* GraphicsDevice::s_Instance = nullptr;

GraphicsDevice::GraphicsDevice()
{
    s_Instance = this;
}

void GraphicsDevice::Init(bool vsync, int antiAlias, bool disableAltEnter)
{
    win = &Window::Get();
    this->vsync = vsync;
    this->antiAlias = antiAlias;
    CreateDeviceAndSwapchain();
    CreateFrameBuffer();
    CreateDepthStencil();
    CreateRasterizerState();
    CreateBlendState();

    if (disableAltEnter) { DisableAltEnter(); }
}

void GraphicsDevice::SetFullscreen(bool set)
{
    if (win->m_isFullscreen == set) { return; }
    swapChain->SetFullscreenState(set, nullptr);
    win->m_isFullscreen = set;
}

void GraphicsDevice::SetRenderTarget()
{
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void GraphicsDevice::Resize()
{
    if (device == nullptr || context == nullptr)
        return;

    context->OMSetRenderTargets(0, 0, 0);

    // Release all outstanding references to the swap chain's buffers.
    SafeRelease(&renderTargetView);

    HRESULT hr;
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(hr));


    // Get buffer and create a render-target-view.
    ID3D11Texture2D* pBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        (void**)&pBuffer);
    assert(SUCCEEDED(hr));

    hr = device->CreateRenderTargetView(pBuffer, NULL,
        &renderTargetView);
    assert(SUCCEEDED(hr));

    SafeRelease(&pBuffer);

    ID3D11Texture2D* pDepthStencil = NULL;
    D3D11_TEXTURE2D_DESC descDepth = {};
    //same width/height as swapchain
    descDepth.Width = win->GetDimensions().x;
    descDepth.Height = win->GetDimensions().y;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = antiAlias;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    if (antiAlias == 1) 
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    else 
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; //MS - multisampling
    descDSV.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(pDepthStencil, &descDSV, &depthStencilView);

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    SafeRelease(&pDepthStencil);
    // Set up the viewport.
    D3D11_VIEWPORT vp;
    vp.Width = win->GetDimensions().x;
    vp.Height = win->GetDimensions().y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);
}

void GraphicsDevice::DisableAltEnter()
{
    HRESULT hr;

    IDXGIDevice* pDXGIDevice;
    hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    assert(SUCCEEDED(hr));

    IDXGIAdapter* pDXGIAdapter;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);
    assert(SUCCEEDED(hr));

    IDXGIFactory* pIDXGIFactory;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);

    pIDXGIFactory->MakeWindowAssociation(win->GetHWnd(), DXGI_MWA_NO_ALT_ENTER);
}

void GraphicsDevice::SwapBuffer()
{
    swapChain->Present(vsync, 0);
}

void GraphicsDevice::ClearBuffer()
{
    float color[4] = { 0.4f, 0.6f, 0.8f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, color);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
}

void GraphicsDevice::CreateDeviceAndSwapchain()
{
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    unsigned int numModes, numerator = 0, denominator = 0;
    size_t stringLength;
    DXGI_ADAPTER_DESC adapterDesc;

    // Create a DirectX graphics interface factory.
    HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to create graphics interface factory");

    // Use the factory to create an adapter for the primary graphics interface (video card).
    result = factory->EnumAdapters(0, &adapter);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to create adapters");


    // Enumerate the primary adapter output (monitor).
    result = adapter->EnumOutputs(0, &adapterOutput);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to enumerate adapters");


    // Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_ENUM_MODES_INTERLACED, &numModes,
        nullptr);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to get modes for diplay");


    // Create a list to hold all the possible display modes for this monitor/video card combination.
    DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];


    // Now fill the display mode list structures.
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_ENUM_MODES_INTERLACED, &numModes,
        displayModeList);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed fill a list of modes");


    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    float refreshRate = 0;
    for (unsigned int i = 0; i < numModes; i++)
    {
        if (displayModeList[i].Width == static_cast<unsigned int>(win->GetMonitorDimensions().x))
        {
            if (displayModeList[i].Height == static_cast<unsigned int>(win->GetMonitorDimensions().y))
            {
                float n = displayModeList[i].RefreshRate.Numerator;
                float d = displayModeList[i].RefreshRate.Denominator;
                if (n / d > refreshRate)
                {
                    refreshRate = n / d;
                    numerator = n;
                    denominator = d;
                }
            }
        }
    }

    // Get the adapter (video card) description.
    result = adapter->GetDesc(&adapterDesc);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to get the description");


    // Store the dedicated video card memory in megabytes.
    videoCardMemory = int(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
    std::string vendor, major, minor, release, build;
    LARGE_INTEGER driverVersion;
    wcstombs_s(NULL, videoCardDescription, 128, adapterDesc.Description, 128);

    if (adapterDesc.VendorId == 0x1002) vendor = "AMD";
    else if (adapterDesc.VendorId == 0x10DE) vendor = "NVIDIA Corporation";
    else if (adapterDesc.VendorId == 0x8086) vendor = "Intel";
    else if (adapterDesc.VendorId == 0x1414) vendor = "Microsoft";
    else                                     vendor = "Unknown vendor!";

    adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion);

    major = std::to_string(HIWORD(driverVersion.HighPart));
    minor = std::to_string(LOWORD(driverVersion.HighPart));
    release = std::to_string(HIWORD(driverVersion.LowPart));
    build = std::to_string(LOWORD(driverVersion.LowPart));

    // Convert the name of the video card to a character array and store it.
    const int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
    LOG_IF_CORE_FATAL(FAILED(result), "Failed to get the name of videocard");
    LOG_CORE_INFO("DirectX Info:");
    LOG_CORE_INFO(" Vendor: %s", vendor.c_str());
    LOG_CORE_INFO(" Renderer: %s", videoCardDescription);
    LOG_CORE_INFO(" Video card memory: %d MB", videoCardMemory);
    LOG_CORE_INFO(" Version: %s.%s.%s.%s", major.c_str(), minor.c_str(), release.c_str(), build.c_str());


    // Release the display mode list.
    delete[] displayModeList;
    displayModeList = nullptr;

    SafeRelease(&adapterOutput);
    SafeRelease(&adapter);
    SafeRelease(&factory);

    DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = numerator;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = denominator;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_descr.SampleDesc.Count = antiAlias; //Antialias can be 1 / 2 / 4 / 8
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descr.BufferCount = 1;
    swap_chain_descr.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_descr.OutputWindow = win->GetHWnd();
    swap_chain_descr.Windowed = !win->IsFullscreen();

    D3D_FEATURE_LEVEL feature_level;
    UINT flags = NULL;// D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swap_chain_descr,
        &swapChain,
        &device,
        &feature_level,
        &context);

    LOG_IF_CORE_INFO(S_OK == hr && swapChain && device && context, "swapChain & device & context created successfully!");
    LOG_IF_CORE_FATAL(!(S_OK == hr && swapChain && device && context), "swapChain & device & context creation Failed!");
}

void GraphicsDevice::CreateFrameBuffer()
{
    ID3D11Texture2D* framebuffer;
    HRESULT hr = swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        (void**)&framebuffer);
    assert(SUCCEEDED(hr));

    hr = device->CreateRenderTargetView(
        framebuffer, 0, &renderTargetView);

    LOG_IF_CORE_INFO(SUCCEEDED(hr), "renderTarget created successfully!");
    LOG_IF_CORE_FATAL(!SUCCEEDED(hr), "renderTarget creation Failed!");

    SafeRelease(&framebuffer);
}

void GraphicsDevice::CreateDepthStencil()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* pDSState;
    HRESULT hr = device->CreateDepthStencilState(&dsDesc, &pDSState);

    //bind
    context->OMSetDepthStencilState(pDSState, 1);

    ID3D11Texture2D* pDepthStencil = NULL;
    D3D11_TEXTURE2D_DESC descDepth = {};
    //same width/height as swapchain
    descDepth.Width = win->GetDimensions().x;
    descDepth.Height = win->GetDimensions().y;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = antiAlias;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);


    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    if (antiAlias == 1)
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    else
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(pDepthStencil, &descDSV, &depthStencilView);

    LOG_IF_CORE_INFO(SUCCEEDED(hr), "depthStencil created successfully!");
    LOG_IF_CORE_FATAL(!SUCCEEDED(hr), "depthStencil creation Failed!");


    SafeRelease(&pDepthStencil);
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void GraphicsDevice::CreateRasterizerState()
{
    RECT winRect;
    GetClientRect(win->GetHWnd(), &winRect);
    D3D11_VIEWPORT viewport = {
      0.0f,
      0.0f,
      (FLOAT)(winRect.right - winRect.left),
      (FLOAT)(winRect.bottom - winRect.top),
      0.0f,
      1.0f };
    context->RSSetViewports(1, &viewport);


    D3D11_RASTERIZER_DESC rasterizerStateDesc;
    rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
    rasterizerStateDesc.FrontCounterClockwise = false;
    rasterizerStateDesc.DepthBias = false;
    rasterizerStateDesc.SlopeScaledDepthBias = 0;
    rasterizerStateDesc.DepthBiasClamp = 0;
    rasterizerStateDesc.DepthClipEnable = true;
    rasterizerStateDesc.ScissorEnable = false;
    rasterizerStateDesc.MultisampleEnable = (antiAlias != 1);
    rasterizerStateDesc.AntialiasedLineEnable = (antiAlias != 1);
    device->CreateRasterizerState(&rasterizerStateDesc, &rasterizerState);
    context->RSSetState(rasterizerState);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GraphicsDevice::CreateBlendState()
{
    D3D11_BLEND_DESC BlendStateDesc;
    ZeroMemory(&BlendStateDesc, sizeof(D3D11_BLEND_DESC));
    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    device->CreateBlendState(&BlendStateDesc, &blendState);

    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(blendState, blendFactor, sampleMask);
}

GraphicsDevice::~GraphicsDevice()
{
    SafeRelease(&blendState);
    SafeRelease(&rasterizerState);
    SafeRelease(&depthStencilView);
    SafeRelease(&renderTargetView);
    SafeRelease(&swapChain);
    SafeRelease(&context);
    SafeRelease(&device);
}