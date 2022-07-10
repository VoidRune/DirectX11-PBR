#include "Application.h"
#include "Graphics/Buffers.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/Texture/HDRCubeMap.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Model.h"
#include "PlayerCamera.h"
#include "Graphics/Geometry.h"
#include "Utility/Time.h"
#include "Material.h"

Application::Application(const char* name, float width, float height) :
	win(new Window),
	device(new GraphicsDevice),
	audioEngine(new AudioEngine)
{
	win->Init(name, false, { width, height });
	device->Init(true, 4, true);
	audioEngine->Init();
}

void Application::Run()
{
	Sound* s = AudioEngine::Get().LoadData("res/Audio/birds.ogg", true);

	Shader shader;
	shader.Create("res/Shaders/PBR.hlsl");
	Shader postProccess;
	postProccess.Create("res/Shaders/postProccess.hlsl");
	Shader sky;
	sky.Create("res/Shaders/sky.hlsl");
	Skybox skybox;

	Material material[4];
	material[0].loadFromPath("res/Material/substance_sphere/gold/");
	material[1].loadFromPath("res/Material/substance_sphere/iron/");
	material[2].loadFromPath("res/Material/substance_sphere/marble/");
	material[3].loadFromPath("res/Material/substance_sphere/wood/");

	Texture ao;
	ao.loadFromFile("res/Material/substance_sphere/ao.png");
	ao.Bind(5);
	Texture BDRF;
	BDRF.loadFromFile("res/Textures/bdrf.png");
	BDRF.Bind(8);

	CubeMapHDR hdrBg;
	hdrBg.loadFromFile("res/Textures/environment.hdr");

	RenderTarget renderTarget;
	renderTarget.Create(Window::Get().GetDimensions().x, Window::Get().GetDimensions().y);
	ScreenQuad quad;

	Model sphere;
	sphere.Create("res/Material/substance_sphere/substance_sphere.obj");

	PlayerCamera playerCam;
	struct cb
	{
		XMMATRIX model;
		XMVECTOR camPos;
	}c;
	ConstantBuffer cBuff;
	cBuff.Create(nullptr, 20 * sizeof(float), 2, CONST_BUFFER_USAGE_VERTEX);

	Time timer;
	AudioEngine::Get().Play2D(s, 0);

	while (win->ProcessInput())
	{
		device->ClearBuffer();
		timer.UpdateDt();
		playerCam.Update(Time::GetDt());


		renderTarget.BindAsTarget();
		
		hdrBg.Bind(0);
		shader.Bind();

		for (int i = 0; i < 4; i++)
		{
			material[i].Bind();
			c.model = XMMatrixTranspose(XMMatrixRotationY(3) * XMMatrixTranslation(i * 3, 0, 0));
			c.camPos = playerCam.GetXMPosition();
			cBuff.SetData(&c, 20 * sizeof(float));
			sphere.Draw();
		}

		sky.Bind();
		skybox.Draw();
		
		GraphicsDevice::Get().SetRenderTarget();
		postProccess.Bind();
		renderTarget.BindAsTexture(0);
		quad.Draw();

		device->SwapBuffer();
	}
}

Application::~Application()
{

}