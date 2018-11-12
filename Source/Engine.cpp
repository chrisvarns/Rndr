#include "Engine.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3dcompiler.h>
#include <sdl/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Imgui/ImguiMenus.h"
#include "Imgui/imgui.h"
#include "D3D11/D3D11RHI.h"
#include "D3D11/D3D11ImguiIntegration.h"
#include "FileUtils.h"
#include "Mesh.h"

using namespace std;

Engine* g_Engine = nullptr;

// Special behavior for ++Colors
RenderMode& operator++(RenderMode& rm) {
	rm = static_cast<RenderMode>(static_cast<int>(rm) + 1);
	if (rm == RenderMode::END_OF_LIST) rm = static_cast<RenderMode>(0);
	return rm;
}

Engine::Engine(int argc, char** argv)
	: m_NumCmdLineArgs(argc)
	, m_CmdLineArgs(argv)
    , window({1280, 720})
	, m_ViewMatrix(1.f)
	, m_ViewAngleV(0.f)
	, m_ViewAngleH(0.f)
	, m_ViewPos(0.f)
	, m_ProjectionMatrix(1.f)
	, m_RenderMode(RenderMode::Normals)
{
    g_Engine = this;
}

Engine::~Engine()
{
	imgui.Shutdown();
}

void Engine::ParseArg(const string& key, const string& value)
{
    if (key == "xres")
    {
        window.width = stoi(value);
    }
    else if (key == "yres")
    {
        window.height = stoi(value);
    }
    else if (key == "scene")
    {
        scenePath = value;
        sceneAssetsBasePath = FileUtils::GetParentDirectory(scenePath);
    }
    else
    {
        SDL_Log("Unknown argument \"%s\".", key.c_str());
        assert(false);
    }
}

void Engine::ParseArgs()
{
    // First read the config file
    auto configFilename = experimental::filesystem::absolute(string(SDL_GetBasePath()) + "../../../../config.txt");
    if (experimental::filesystem::exists(configFilename))
    {
        auto fileData = FileUtils::LoadFileAbsolute(configFilename.string());
        string config = string(fileData.begin(), fileData.end());
        stringstream ss(config);
        string key;
        while (getline(ss, key, '='))
        {
            string value;
            assert(getline(ss, value));
            ParseArg(key, value);
        }
    }

    // Then parse cmd line args
	for (int i = 1; i < m_NumCmdLineArgs; ++i)
	{
		string cmdLineArg = m_CmdLineArgs[i];
        stringstream ss(cmdLineArg);
		//split the string
        string key;
        string value;
        getline(ss, key, '=');
        getline(ss, value);

        ParseArg(key, value);
	}
}

bool Engine::Init()
{
    ParseArgs();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Unable to init Video: %s", SDL_GetError());
		return false;
	}

    // TODO Move this code to the window class
	window.sdlWindow.reset(SDL_CreateWindow(
		"Rndr",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window.width, window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

	if (!window.sdlWindow) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		return false;
	}

	ZeroMemory(&window.m_SdlWindowWMInfo, sizeof(SDL_SysWMinfo));
	if (!SDL_GetWindowWMInfo(window.sdlWindow.get(), &window.m_SdlWindowWMInfo))
	{
		SDL_Log("SDL_GetWindowWMInfo failed.");
		return false;
	}

    rhi.InitRHI(window);

	imgui.Init(window.sdlWindow.get(), &rhi);

	// TODO Feels a bit out of place here.
    UpdateProjectionMatrix();

	SetupRenderTargets();

	return true;
}

void Engine::SetupRenderTargets()
{
	RenderTargetCreateInfo rtCreateInfo;
	rtCreateInfo.width = window.width;
	rtCreateInfo.height = window.height;
	// Color
	m_Gbuffers.push_back(rhi.CreateRenderTarget(rtCreateInfo));
	// Normal
	m_Gbuffers.push_back(rhi.CreateRenderTarget(rtCreateInfo));
}

bool Engine::Execute()
{
	while (true)
	{
		// Handle events first
		if (!HandleEvents()) break;

		// Update state
		Update(1.f);

		// Render
		Render();
	}

	return true;
}

void Engine::UpdateProjectionMatrix()
{
	m_ProjectionMatrix = glm::perspective(45.f, (float)window.width / window.height, 0.01f, 100.f);
}

void Engine::ResizeWindow(int width, int height)
{
	window.width = width;
	window.height = height;

    rhi.HandleWindowResize(width, height);

    UpdateProjectionMatrix();
}

void Engine::HandleWindowEvent(const SDL_Event& event)
{
	switch (event.window.event)
	{
	case SDL_WINDOWEVENT_RESIZED:
		ResizeWindow(event.window.data1, event.window.data2);
		break;
	default:
		break;
	}
}

bool Engine::HandleEvents()
{
	ImGuiIO io = ImGui::GetIO();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		imgui.ProcessEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			SDL_Log("SDL_QUIT");
			return false;
			break;

		case SDL_WINDOWEVENT:
			HandleWindowEvent(event);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (!io.WantCaptureMouse && event.button.button == SDL_BUTTON_RIGHT) SDL_SetRelativeMouseMode(SDL_TRUE);
			break;
		case SDL_MOUSEBUTTONUP:
			if (!io.WantCaptureMouse && event.button.button == SDL_BUTTON_RIGHT) SDL_SetRelativeMouseMode(SDL_FALSE);
			break;
		case SDL_MOUSEMOTION:
			if (!io.WantCaptureMouse && SDL_GetRelativeMouseMode())
			{
				m_ViewAngleH += event.motion.xrel * ImGui::Integration::g_Controls_Camera_Sensitivity;
				m_ViewAngleV -= event.motion.yrel * ImGui::Integration::g_Controls_Camera_Sensitivity;
			}
			break;
		case SDL_KEYDOWN:
			if (!io.WantCaptureMouse)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_r:
					++m_RenderMode;
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	return true;
}

bool Engine::LoadContent()
{
	// Load the asset with assimp
	Assimp::Importer assimp;
	const aiScene* pScene = assimp.ReadFile(scenePath,
		(aiProcess_ConvertToLeftHanded	// Convert to CW for DirectX.
		| aiProcessPreset_TargetRealtime_MaxQuality)
	);

	for (uint32_t meshIdx = 0; meshIdx < pScene->mNumMeshes; ++meshIdx)
	{
		const aiMesh& aimesh = *pScene->mMeshes[meshIdx];
		SharedDeletePtr<Mesh> mesh = Mesh::LoadMesh(aimesh, *pScene, rhi);
		m_Meshes.push_back(mesh);
	}

    rhi.LoadVertexShader();
    rhi.LoadPixelShader();

	return true;
}

bool Engine::UpdateCamera(float deltaTime)
{
	// Wrap around/clamp the view angles
	if (m_ViewAngleH > glm::two_pi<float>()) m_ViewAngleH -= glm::two_pi<float>();
	else if (m_ViewAngleH < 0) m_ViewAngleH += glm::two_pi<float>();
	m_ViewAngleV = glm::clamp(m_ViewAngleV, glm::radians(-85.f), glm::radians(85.f));

	glm::vec3 viewDir(
		glm::cos(m_ViewAngleV) * glm::sin(m_ViewAngleH),
		glm::sin(m_ViewAngleV),
		glm::cos(m_ViewAngleV) * glm::cos(m_ViewAngleH)
	);

	glm::vec3 rightDir(
		glm::sin(m_ViewAngleH + glm::half_pi<float>()),
		0,
		glm::cos(m_ViewAngleH + glm::half_pi<float>())
	);

	glm::vec3 upDir = glm::cross(viewDir, rightDir);

	// If the RMB is held, we can grab WASD and update the camera pos.
	if (SDL_GetRelativeMouseMode())
	{
		const uint8_t* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_W]) m_ViewPos += viewDir * ImGui::Integration::g_Controls_Camera_Speed;
		if (keyboardState[SDL_SCANCODE_S]) m_ViewPos -= viewDir * ImGui::Integration::g_Controls_Camera_Speed;
		if (keyboardState[SDL_SCANCODE_A]) m_ViewPos -= rightDir * ImGui::Integration::g_Controls_Camera_Speed;
		if (keyboardState[SDL_SCANCODE_D]) m_ViewPos += rightDir * ImGui::Integration::g_Controls_Camera_Speed;
		if (keyboardState[SDL_SCANCODE_E]) m_ViewPos += upDir * ImGui::Integration::g_Controls_Camera_Speed;
		if (keyboardState[SDL_SCANCODE_Q]) m_ViewPos -= upDir * ImGui::Integration::g_Controls_Camera_Speed;
	}

	m_ViewMatrix = glm::lookAt(m_ViewPos, m_ViewPos + viewDir, upDir);
	return true;
}

bool Engine::Update(float deltaTime)
{
	imgui.NewFrame(window.sdlWindow.get());

	UpdateCamera(deltaTime);

    auto viewProjMatrix = m_ProjectionMatrix * m_ViewMatrix;

	for (auto& meshItr : m_Meshes)
	{
		ConstantBufferData constBuffer;
		constBuffer.mvpMatrix =  viewProjMatrix * meshItr->modelMatrix;
		constBuffer.renderMode = glm::ivec4(static_cast<int>(m_RenderMode));

		// Update the constant buffer...
        rhi.UpdateConstantBuffer(meshItr->constantBuffer, constBuffer);
	}

	return true;
}

bool Engine::Render()
{
    std::array<float, 4> clearColor = { 0.f, 0.f, 0.25f, 1.f };
    rhi.ClearBackBuffer(clearColor);
    rhi.SetVertexShader();
    rhi.SetPixelShader();

	for (const auto& meshItr : m_Meshes)
	{
        rhi.DrawMesh(*meshItr);
	}

	ImGui::Integration::RenderMenus();
	ImGui::Render();

    rhi.Present();

	return true;
}