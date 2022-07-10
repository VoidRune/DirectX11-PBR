#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	Application* app = new Application("Application", 1280, 720);
	app->Run();
	delete app;
	return 0;
}