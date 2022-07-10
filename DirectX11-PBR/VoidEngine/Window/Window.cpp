#include "Window/Window.h"
//#include "Graphics/GraphicsDevice.h"
#include <windowsx.h>
//#include "ImGui/ImGuiImpl.h"
//#include "resource.h"

Window* Window::s_Instance = nullptr;

Window::Window() : hInstance(GetModuleHandle(nullptr))
{
    s_Instance = this;
}

void Window::Init(std::string title, bool isFullscreen, vec2f dimensions)
{
    LOG_INIT();

    m_monitorDimensions.x = GetSystemMetrics(SM_CXSCREEN);
    m_monitorDimensions.y = GetSystemMetrics(SM_CYSCREEN);
    (isFullscreen) ? m_dimensions = m_monitorDimensions : m_dimensions = dimensions;
    m_title = title;
    m_isFullscreen = isFullscreen;

    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEXA));

    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
#ifdef IDI_ICON1
    wc.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
    wc.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));
#else
    wc.hIcon = nullptr;
    wc.hIconSm = nullptr;
#endif // IDI_ICON1

#ifdef IDC_CURSOR1
    wc.hCursor = static_cast<HCURSOR>(LoadImage(hInstance, MAKEINTRESOURCE(IDC_CURSOR1), IMAGE_CURSOR, 24, 24, 0));
#else
    wc.hCursor = nullptr;
#endif // IDC_CURSOR1

    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = title.c_str();


    RegisterClassExA(&wc);

    RECT wr = { 0, 0, m_dimensions.x, m_dimensions.y };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
    hWnd = CreateWindowExA(
        0,
        title.c_str(),
        (LPCSTR)(std::wstring(title.begin(), title.end())).c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (m_monitorDimensions.x - m_dimensions.x) / 2, (m_monitorDimensions.y - m_dimensions.y) / 2,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr,
        nullptr,
        hInstance,
        this);

    LOG_CORE_INFO("Window created successfully!");
    ShowWindow(hWnd, SW_SHOW);
}

int Window::ProcessInput()
{
    Input::Update();
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            return 0;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 1;
}

void Window::SetCursorVisibility(const bool& visible)
{
    ShowCursor(visible);
}

bool Window::IsCursorVisible()
{
    CURSORINFO ci = { sizeof(CURSORINFO) };
    if (GetCursorInfo(&ci))
        return ci.flags & CURSOR_SHOWING;
    return false;
}

void Window::ResizeWindow(uint32_t width, uint32_t height)
{
    if (m_dimensions.x == width && m_dimensions.y == height) { return; }
    else if (m_isFullscreen) { return; }
    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
    SetWindowPos(hWnd, HWND_TOP,
        m_monitorDimensions.x / 2.0f - (wr.right - wr.left) / 2.0f,
        m_monitorDimensions.y / 2.0f - (wr.bottom - wr.top) / 2.0f,
        wr.right - wr.left, wr.bottom - wr.top, 0);
    SetFocus(hWnd);
}

void Window::CloseWindow()
{
    PostQuitMessage(0);
}

Window::~Window()
{
    UnregisterClassA("Window", hInstance);
    DestroyWindow(hWnd);
}

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    //    return true;

    switch (message)
    {
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        Input::mouseButtonStates[0] = true;
        SetCapture(hWnd);
        break;
    }
    case WM_LBUTTONUP:
    {
        Input::mouseButtonStates[0] = false;
        ReleaseCapture();
        break;
    }
    case WM_RBUTTONDOWN:
    {
        Input::mouseButtonStates[1] = true;
        break;
    }
    case WM_RBUTTONUP:
    {
        Input::mouseButtonStates[1] = false;
        break;
    }
    case WM_MOUSEMOVE:
    {
        Input::mouseHasMoved = true;
        Input::mousePosition = { (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam) };
        break;
    }
    case WM_MOUSEWHEEL:
    {
        Input::mouseScroll = GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    }
    case WM_KEYDOWN:
    {
        //if (wParam == VK_ESCAPE)
        //{
        //    PostQuitMessage(0);
        //    return 0;
        //}
        Input::keyStates[static_cast<unsigned char>(wParam)] = true;
        break;
    }
    case WM_KEYUP:
    {
        Input::keyStates[static_cast<unsigned char>(wParam)] = false;
        break;
    }
    case WM_SIZE:
    {
        m_dimensions.x = LOWORD(lParam);
        m_dimensions.y = HIWORD(lParam);

        //if (&GraphicsDevice::Get() != nullptr) { GraphicsDevice::Get().Resize(); }

        break;
    }
    case WM_CHAR:
    {
        if (Input::keyBufferIndex < 8)
        {
            Input::keyBuffer[Input::keyBufferIndex] = wParam;
            Input::keyBufferIndex++;
        }
        break;
    }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (msg == WM_NCCREATE)
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        // set WinAPI-managed user data to store ptr to window instance
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        // forward message to window instance handler
        return pWnd->WindowProc(hWnd, msg, wParam, lParam);
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // retrieve ptr to window instance
    Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    // forward message to window instance handler
    return pWnd->WindowProc(hWnd, msg, wParam, lParam);
}