#pragma once
#include <memory>
#include "Window/Window.h"
#include "Graphics/GraphicsDevice.h"
#include "Audio/AudioEngine.h"

class Application
{
public:
	Application(const char* name, float width, float height);
	~Application();

	void Run();
private:

	/* Core */
	std::unique_ptr<Window> win;
	std::unique_ptr<GraphicsDevice> device;
	//std::unique_ptr<Renderer> renderer;
	std::unique_ptr<AudioEngine> audioEngine;

	/* Game */
	//int gameState;
	//MainMenu* mainMenu;
	//Game* game;
};