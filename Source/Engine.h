#pragma once

#include <string>
#include <vector>
#include <sdl/SDL.h>
#include <sdl/SDL_syswm.h>
#include <d3d11.h>
#include "UniquePtr.h"
#include "SharedPtr.h"
#include "Mesh.h"
#include <assimp/scene.h>
#include <glm/glm.hpp>

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
	Engine(int argc, char** argv);
	~Engine();
	bool Init();
	bool LoadContent();
	bool Execute();

private:
	static std::vector<std::string> ms_Commands;
	int				m_NumCmdLineArgs;
	char**			m_CmdLineArgs;
	int				m_WindowWidth;
	int				m_WindowHeight;
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
	UniqueReleasePtr<ID3D11Texture2D>			m_pDepthStencilRT;
	UniqueReleasePtr<ID3D11DepthStencilView>	m_pDepthStencilRTView;
	UniqueReleasePtr<ID3D11DepthStencilState>	m_pDepthStencilState;
	UniqueReleasePtr<ID3D11RasterizerState>		m_pRasterState;
	UniqueReleasePtr<ID3D11InputLayout>			m_pInputLayout;
	UniqueReleasePtr<ID3D11VertexShader>		m_pSolidColourVs;
	UniqueReleasePtr<ID3D11PixelShader>			m_pSolidColourPs;

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

};

