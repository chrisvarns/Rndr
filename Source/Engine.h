#pragma once

#include <string>
#include <vector>

#include <sdl/SDL.h>
#include <sdl/SDL_syswm.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "UniquePtr.h"
#include "SharedPtr.h"
#include "Window.h"
#include "Mesh.h"

namespace RHI {
class RHI;
}

class ImguiIntegration;

enum RenderMode {
	SolidColour = 0,
	Normals,
	UVs,
	Depth,
	END_OF_LIST
};

RenderMode& operator++(RenderMode& rm);
RenderMode operator++(RenderMode& rm, int);

class Engine
{
public:
    static Engine* g_Engine;
	Engine(int argc, char** argv);
	~Engine();
	bool Init();
	bool LoadContent();
	bool Execute();

	static std::vector<std::string> ms_Commands;
	int				m_NumCmdLineArgs;
	char**			m_CmdLineArgs;

	std::string		scenePath;
    std::string     sceneAssetsBasePath;
    std::string     workingDir;

    Window          window;

    std::unique_ptr<RHI::RHI>                   rhi;
    std::unique_ptr<ImguiIntegration>          imgui;

	std::vector<SharedPtr<Mesh> >				m_Meshes;

	// Camera stuff
	glm::mat4									m_ViewMatrix;
	float										m_ViewAngleH;
	float										m_ViewAngleV;
	glm::vec3									m_ViewPos;
	glm::mat4									m_ProjectionMatrix;
	
	// Render Stuff
	RenderMode									m_RenderMode;

	bool ParseArgs();
	
	bool HandleEvents();
	bool Update(float deltaTime);
	bool UpdateCamera(float deltaTime);
	bool Render();
	
	void ResizeWindow(int width, int height);
	void HandleWindowEvent(const SDL_Event& event);
	void UpdateProjectionMatrix();
};

