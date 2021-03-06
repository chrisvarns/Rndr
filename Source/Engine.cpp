#include "Engine.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <d3d11_1.h>
#include <d3d11sdklayers.h>
#include <d3dcompiler.h>
#include <sdl/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "ImguiMenus.h"
#include "Imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/ImGuizmo.h"

#include "D3D11RHI.h"
#include "FileUtils.h"
#include "Mesh.h"

namespace fs = std::experimental::filesystem;

Engine* g_Engine = nullptr;

Camera::Camera()
	: viewMatrix(1.f)
	, projectionMatrix(1.f)
	, viewPos(0.f)
	, viewDir(0.f)
	, upDir(0.f)
	, rightDir(0.f)
	, viewAngleH(0.f)
	, viewAngleV(0.f)
{}

// Special behavior for ++Colors
RenderMode& operator++(RenderMode& rm) {
	rm = static_cast<RenderMode>(static_cast<int>(rm) + 1);
	if (rm == RenderMode::END_OF_LIST) rm = static_cast<RenderMode>(0);
	return rm;
}

Engine::Engine(int argc, char** argv)
	: CommandLineArgs(argv+1, argv+argc)
    , window({1280, 720})
	, m_RenderMode(RenderMode::Albedo)
{
    g_Engine = this;
}

Engine::~Engine()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Engine::ParseArg(const std::string& key, const std::string& value)
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
        ScenePath = (fs::path(ProjectDir) / value).string();
        SceneAssetsBaseDir = FileUtils::GetParentDirectory(ScenePath);
    }
    else
    {
        SDL_Log("Unknown argument \"%s\".", key.c_str());
        assert(false);
    }
}

void Engine::ParseArgs()
{
	ProjectDir = fs::canonical(std::string(SDL_GetBasePath()) + "../../../../").string();

	fs::path ConfigFilename = fs::path(ProjectDir) / "config.txt";

    if (fs::exists(ConfigFilename))
    {
        auto fileData = FileUtils::LoadFileAbsolute(ConfigFilename.string());
        std::string config = std::string(fileData.begin(), fileData.end());
        std::stringstream ss(config);
        std::string key;
        while (getline(ss, key, '='))
        {
            std::string value;
            assert(getline(ss, value));
            ParseArg(key, value);
        }
    }

    // Then parse cmd line args
	for (const std::string& CommandLineArg : CommandLineArgs)
	{
		std::stringstream ss(CommandLineArg);
		//split the string
		std::string key;
		std::string value;
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

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForDX11(window.sdlWindow.get());
	ImGui_ImplDX11_Init(rhi.GetDevice(), rhi.GetDeviceContext());
	ImGui::StyleColorsDark();

	// TODO Feels a bit out of place here.
    UpdateProjectionMatrix();

	return true;
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
	camera.projectionMatrix = glm::perspective(45.f, (float)window.width / window.height, 0.01f, 100.f);
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
		ImGui_ImplSDL2_ProcessEvent(&event);
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
				camera.viewAngleH += event.motion.xrel * Globals::CameraSensitivity;
				camera.viewAngleV -= event.motion.yrel * Globals::CameraSensitivity;
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
	const aiScene* pScene = assimp.ReadFile(ScenePath,
		(aiProcess_ConvertToLeftHanded	// Convert to CW for DirectX.
		| aiProcessPreset_TargetRealtime_MaxQuality)
	);

	for (uint32_t meshIdx = 0; meshIdx < pScene->mNumMeshes; ++meshIdx)
	{
		const aiMesh& aimesh = *pScene->mMeshes[meshIdx];
		SharedDeletePtr<Mesh> mesh = Mesh::LoadMesh(aimesh, *pScene, rhi);
		m_Meshes.push_back(mesh);
	}

	return true;
}

bool Engine::UpdateCamera(float deltaTime)
{
	// Wrap around/clamp the view angles
	if (camera.viewAngleH > glm::two_pi<float>()) camera.viewAngleH -= glm::two_pi<float>();
	else if (camera.viewAngleH < 0) camera.viewAngleH += glm::two_pi<float>();
	camera.viewAngleV = glm::clamp(camera.viewAngleV, glm::radians(-85.f), glm::radians(85.f));

	camera.viewDir = glm::vec3(
		glm::cos(camera.viewAngleV) * glm::sin(camera.viewAngleH),
		glm::sin(camera.viewAngleV),
		glm::cos(camera.viewAngleV) * glm::cos(camera.viewAngleH)
	);

	camera.rightDir = glm::vec3(
		glm::sin(camera.viewAngleH + glm::half_pi<float>()),
		0,
		glm::cos(camera.viewAngleH + glm::half_pi<float>())
	);

	camera.upDir = glm::cross(camera.viewDir, camera.rightDir);

	// If the RMB is held, we can grab WASD and update the camera pos.
	if (SDL_GetRelativeMouseMode())
	{
		const uint8_t* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_W]) camera.viewPos += camera.viewDir * Globals::CameraSpeed;
		if (keyboardState[SDL_SCANCODE_S]) camera.viewPos -= camera.viewDir * Globals::CameraSpeed;
		if (keyboardState[SDL_SCANCODE_A]) camera.viewPos -= camera.rightDir * Globals::CameraSpeed;
		if (keyboardState[SDL_SCANCODE_D]) camera.viewPos += camera.rightDir * Globals::CameraSpeed;
		if (keyboardState[SDL_SCANCODE_E]) camera.viewPos += camera.upDir * Globals::CameraSpeed;
		if (keyboardState[SDL_SCANCODE_Q]) camera.viewPos -= camera.upDir * Globals::CameraSpeed;
	}

	camera.viewMatrix = glm::lookAt(camera.viewPos, camera.viewPos + camera.viewDir, camera.upDir);
	return true;
}

bool Engine::Update(float deltaTime)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(window.sdlWindow.get());
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	UpdateCamera(deltaTime);

    auto viewProjMatrix = camera.projectionMatrix * camera.viewMatrix;

	for (auto& meshItr : m_Meshes)
	{
		GeometryConstantBufferLayout constBuffer;
		constBuffer.mvpMatrix =  viewProjMatrix * meshItr->modelMatrix;

		// Update the constant buffer...
        rhi.UpdateConstantBuffer(meshItr->constantBuffer, &constBuffer, sizeof(constBuffer));
	}

	return true;
}

bool Engine::Render()
{
	rhi.BeginGeometryPass();

	for (const auto& meshItr : m_Meshes)
	{
        rhi.DrawMesh(*meshItr);
	}

	rhi.BeginLightingPass();
	rhi.DrawAmbient(Globals::LightingAmbientColor);
	rhi.DrawDirectionalLight(Globals::LightingDirectionalColor, Globals::LightingDirectionalRot);

	ImGui::Integration::RenderMenus();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    rhi.Present();

	return true;
}