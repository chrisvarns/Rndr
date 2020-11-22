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
#include "D3D11RHI.h"

struct GBuffers {
	ID3D11Texture2D* color;
	ID3D11Texture2D* normal;
};

enum RenderMode {
	Albedo = 0,
	Normals,
	END_OF_LIST
};

RenderMode& operator++(RenderMode& rm);
RenderMode operator++(RenderMode& rm, int);

struct Camera {
	glm::mat4									viewMatrix;
	glm::mat4									projectionMatrix;
	glm::vec3									viewPos;
	glm::vec3									viewDir;
	glm::vec3									upDir;
	glm::vec3									rightDir;
	float										viewAngleH;
	float										viewAngleV;

	Camera();
};

class Engine
{
public:

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

	std::vector<char*> CommandLineArgs;

	std::string	ScenePath;
    std::string SceneAssetsBaseDir;
	std::string ProjectDir;

    Window          window;

    D3D11RHI        rhi;

	std::vector<SharedPtr<Mesh>>				m_Meshes;
    TextureMap                                  textureMap;

	Camera camera;
	
	// Render Stuff
	RenderMode									m_RenderMode;

private:
    void ParseArg(const std::string& key, const std::string& value);
};

extern Engine* g_Engine;
