#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "UniquePtr.h"

struct VertexDesc
{
	DirectX::XMFLOAT3 Position;
	//DirectX::XMFLOAT4 colour;
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
	int				m_NumCmdLineArgs;
	char**			m_CmdLineArgs;
	int				m_WindowWidth;
	int				m_WindowHeight;
	std::string		m_ShaderDir;

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

	// Mesh stuff
	UniqueReleasePtr<ID3D11Buffer>				m_pVertexBuffer;
	UniqueReleasePtr<ID3D11InputLayout>			m_pInputLayout;
	UniqueReleasePtr<ID3D11VertexShader>		m_pSolidColourVs;
	UniqueReleasePtr<ID3D11PixelShader>			m_pSolidColourPs;

	static std::vector<std::string> ms_Commands;
	int ParseArgs();

	int HandleEvents();
	int Update(FLOAT DeltaTime);
	int Render();
};

