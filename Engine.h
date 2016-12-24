#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sdl/SDL.h>
#include <sdl/SDL_syswm.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "UniquePtr.h"
#include <assimp/scene.h>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])

enum RenderMode {
	SolidColour = 0,
	Normals,
	Depth,
	END_OF_LIST
};

RenderMode& operator++(RenderMode& rm);
RenderMode operator++(RenderMode& rm, int);

struct ConstBuffer
{
	glm::mat4 mvpMatrix;
	glm::ivec4 renderMode;
};

class Engine
{
public:
	Engine(int argc, char** argv);
	~Engine();
	int Init();
	int LoadContent();
	int Execute();
	int Release();

private:
	static std::vector<std::string> ms_Commands;
	int				m_NumCmdLineArgs;
	char**			m_CmdLineArgs;
	int				m_WindowWidth;
	int				m_WindowHeight;
	std::string		m_ShaderDir;
	std::string		m_MeshPath;

	// SDL
	UniquePtr<SDL_Window> m_pSdlWindow = UniquePtr<SDL_Window>([](SDL_Window* window) { SDL_DestroyWindow(window); });
	SDL_SysWMinfo m_SdlWindowWMInfo;

	// D3D
	UniqueReleasePtr<IDXGIAdapter1>				m_pAdapter;
	UniqueReleasePtr<IDXGISwapChain>			m_pSwapChain;
	UniqueReleasePtr<ID3D11Device>				m_pD3dDevice;
	UniqueReleasePtr<ID3D11DeviceContext>		m_pD3dContext;
	UniqueReleasePtr<ID3D11Texture2D>			m_pBackBufferRT;
	UniqueReleasePtr<ID3D11RenderTargetView>	m_pBackBufferRTView;
	UniqueReleasePtr<ID3D11RasterizerState>		m_pRasterState;

	// Mesh stuff
	UniqueReleasePtr<ID3D11Buffer>				m_pVertexBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pNormalBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pIndexBuffer;
	UniqueReleasePtr<ID3D11InputLayout>			m_pInputLayout;
	UniqueReleasePtr<ID3D11VertexShader>		m_pSolidColourVs;
	UniqueReleasePtr<ID3D11PixelShader>			m_pSolidColourPs;
	int											m_pNumVerts;
	glm::mat4									m_ModelMatrix;

	// Camera stuff
	UniqueReleasePtr<ID3D11Buffer>				m_pConstantBuffer;
	glm::mat4									m_ViewMatrix;
	float										m_ViewAngleH;
	float										m_ViewAngleV;
	glm::vec3									m_ViewPos;
	glm::mat4									m_ProjectionMatrix;
	
	// Render Stuff
	RenderMode									m_RenderMode;

	int ParseArgs();
	int HandleEvents();
	int Update(float deltaTime);
	int UpdateCamera(float deltaTime);
	int Render();
};

