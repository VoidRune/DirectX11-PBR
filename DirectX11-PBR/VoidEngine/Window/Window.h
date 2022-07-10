#pragma once
#define _WINSOCKAPI_
#define _WINSOCK2API_
#include <Windows.h>
#undef _WINSOCK2API_
#undef _WINSOCKAPI_
#include <string>
#include "Utility/Log.h"
#include "Window/Input.h"

class Window
{
public:
	friend class GraphicsDevice;
	static inline Window& Get() { return *s_Instance; }
	Window();
	void Init(std::string title = "Application", bool isFullscreen = false, vec2f dimensions = { 1280, 720 });
	~Window();

	int ProcessInput();
	void SetCursorVisibility(const bool& visible);
	bool IsCursorVisible();
	void ResizeWindow(uint32_t width, uint32_t height);
	void CloseWindow();


	HWND GetHWnd() { return hWnd; };
	HINSTANCE GetHInstance() { return hInstance; };
	bool IsFullscreen() { return m_isFullscreen; };
	vec2f GetDimensions() { return m_dimensions; };
	vec2f GetMonitorDimensions() { return m_monitorDimensions; };

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
private:
	HINSTANCE hInstance;
	HCURSOR hCursor;
	HWND hWnd;

	std::string m_title;
	vec2f m_dimensions;
	vec2f m_monitorDimensions;
	bool m_isFullscreen;

	static Window* s_Instance;

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};