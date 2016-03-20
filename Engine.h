#pragma once

#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>

using namespace std;

class Engine
{
public:
	Engine(int argc, char** argv);
	~Engine();
	int Init();
	int Execute();

private:
	int		m_NumCmdLineArgs;
	char**	m_CmdLineArgs;
	int		m_WindowWidth;
	int		m_WindowHeight;

	// SDL
	SDL_Window*		m_pSdlWindow;
	SDL_SysWMinfo	m_SdlWindowWMInfo;

	// D3D
	IDXGIAdapter1*			m_pAdapter;
	IDXGISwapChain*			m_pSwapChain;
	ID3D11Device*			m_pD3dDevice;
	ID3D11DeviceContext*	m_pD3dContext;
	ID3D11Texture2D*		m_pBackBufferRT;
	ID3D11RenderTargetView*	m_pBackBufferRTView;

	/// Statics
private:
	static vector<string> ms_Commands;
	int ParseArgs();

	int HandleEvents();
	int Update(FLOAT DeltaTime);
	int Render();

};

