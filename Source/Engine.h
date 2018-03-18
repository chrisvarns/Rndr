#pragma once

#include <string>
#include <vector>

#include <sdl/SDL.h>
#include <sdl/SDL_syswm.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "SharedPtr.h"
#include "TextureMap.h"
#include "UniquePtr.h"
#include "Window.h"

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
    void ParseArgs();
    bool HandleEvents();
    bool Update(float deltaTime);
    bool UpdateCamera(float deltaTime);
    bool Render();
    void ResizeWindow(int width, int height);
    void HandleWindowEvent(const SDL_Event& event);
    void UpdateProjectionMatrix();

	int				m_NumCmdLineArgs;
	char**			m_CmdLineArgs;
	std::string		scenePath;
    std::string     sceneAssetsBasePath;

    Window          window;

    std::unique_ptr<RHI::RHI>                   rhi;
    std::unique_ptr<ImguiIntegration>           imgui;

	std::vector<SharedPtr<Mesh> >				m_Meshes;
	std::vector<RHI::RHIRenderTargetHandle>		m_Gbuffers;
    TextureMap                                  textureMap;

	// Camera stuff
	glm::mat4									m_ViewMatrix;
	float										m_ViewAngleH;
	float										m_ViewAngleV;
	glm::vec3									m_ViewPos;
	glm::mat4									m_ProjectionMatrix;
	
	// Render Stuff
	RenderMode									m_RenderMode;

private:
    void ParseArg(const std::string& key, const std::string& value);
	void SetupRenderTargets();
};

