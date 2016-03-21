#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	//DirectX::XMFLOAT4 colour;
};

template <typename T>
class UniqueReleasePtr : public std::unique_ptr<T, void(*)(T* ptr)>
{
public:

	// Default constructor, sets up Release destructor
	UniqueReleasePtr() : std::unique_ptr<T, void(*)(T* ptr)>(nullptr, [](T* ptr){ ptr->Release(); }) {};

	// Constructor taking a specific deleter function to be called on destruction
	UniqueReleasePtr(void(deleter)(T* ptr)) : std::unique_ptr<T, void(*)(T* ptr)>(nullptr, deleter) {};

	// Returns a pointer to the internal pointer storage.
	T** GetRef() { return reinterpret_cast<T**>(&_Mypair._Get_second()); }
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
	int		m_NumCmdLineArgs;
	char**	m_CmdLineArgs;
	int		m_WindowWidth;
	int		m_WindowHeight;

	// SDL
	UniqueReleasePtr<SDL_Window> m_pSdlWindow = UniqueReleasePtr<SDL_Window>([](SDL_Window* window) { SDL_DestroyWindow(window); });
	SDL_SysWMinfo m_SdlWindowWMInfo;

	// D3D
	UniqueReleasePtr<IDXGIAdapter1>				m_pAdapter;
	UniqueReleasePtr<IDXGISwapChain>			m_pSwapChain;
	UniqueReleasePtr<ID3D11Device>				m_pD3dDevice;
	UniqueReleasePtr<ID3D11DeviceContext>		m_pD3dContext;
	UniqueReleasePtr<ID3D11Texture2D>			m_pBackBufferRT;
	UniqueReleasePtr<ID3D11RenderTargetView>	m_pBackBufferRTView;

	static std::vector<std::string> ms_Commands;
	int ParseArgs();

	int HandleEvents();
	int Update(FLOAT DeltaTime);
	int Render();
};

