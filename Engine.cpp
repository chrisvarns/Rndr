#include "Engine.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <string>
#include <sstream>
#include <d3d11.h>

using namespace std;

vector<string> Engine::ms_Commands = { "xres", "yres", "scene" };

Engine::Engine(int argc, char** argv)
	: m_NumCmdLineArgs(argc)
	, m_CmdLineArgs(argv)
	, m_WindowWidth(1280)
	, m_WindowHeight(720)
	, m_pAdapter(NULL)
	, m_pD3dDevice(NULL)
	, m_pD3dContext(NULL)
{
}

Engine::~Engine()
{
	// Release D3D objects in reverse order
	if (m_pBackBufferRTView)	m_pBackBufferRTView->Release();
	if (m_pBackBufferRT)		m_pBackBufferRT->Release();
	if (m_pD3dContext)			m_pD3dContext->Release();
	if (m_pD3dDevice)			m_pD3dDevice->Release();
	if (m_pSwapChain)			m_pSwapChain->Release();
	if (m_pAdapter)				m_pAdapter->Release();

	// Release any SDL objects
	if (m_pSdlWindow)			SDL_DestroyWindow(m_pSdlWindow);
}

int Engine::ParseArgs()
{
	for (int i = 1; i < m_NumCmdLineArgs; ++i)
	{
		string cmd = m_CmdLineArgs[i];

		//split the string
		size_t mid = cmd.find("=");
		string arg = cmd.c_str() + mid + 1;
		cmd.resize(mid);

		if (std::find(ms_Commands.begin(), ms_Commands.end(), cmd) == ms_Commands.end())
			return 1;

		if (cmd == "xres")
		{
			m_WindowWidth = stoi(arg);
		}
		else if (cmd == "yres")
		{
			m_WindowHeight = stoi(arg);
		}
	}
	return 0;
}

int Engine::Init()
{
	if (ParseArgs())
	{
		SDL_Log("Failed to parse args");
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Unable to init Video: %s", SDL_GetError());
		return 2;
	}

	m_pSdlWindow = SDL_CreateWindow(
		"Rndr",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_WindowWidth, m_WindowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (!m_pSdlWindow) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		return 3;
	}

	ZeroMemory(&m_SdlWindowWMInfo, sizeof(SDL_SysWMinfo));
	if (!SDL_GetWindowWMInfo(m_pSdlWindow, &m_SdlWindowWMInfo))
	{
		SDL_Log("SDL_GetWindowWMInfo failed.");
		return 4;
	}

	IDXGIFactory1* pFactory = NULL;
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory))))
	{
		SDL_Log("CreateDXGIFactory1 failed.");
		return 5;
	}

	IDXGIAdapter1* pTmpAdapter = NULL;
	for (UINT i = 0; pFactory->EnumAdapters1(i, &pTmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		pTmpAdapter->GetDesc1(&adapterDesc);

		// Get the nvidia adapter
		if (adapterDesc.VendorId == 4318)
		{
			m_pAdapter = pTmpAdapter;
			break;
		}
	}
	pFactory->Release();
	pFactory = NULL;
	if (m_pAdapter == NULL)
	{
		SDL_Log("Failed to find NVidia device!!.");
		return 6;
	};

	// Set up the device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = m_WindowWidth;
	swapChainDesc.BufferDesc.Height = m_WindowHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = m_SdlWindowWMInfo.info.win.window;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		m_pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, creationFlags,
		featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc,
		&m_pSwapChain, &m_pD3dDevice, &featureLevel, &m_pD3dContext)))
	{
		SDL_Log("D3D11CreateDeviceAndSwapChain failed.");
		return 7;
	}
	
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(m_pBackBufferRT), (void**)&m_pBackBufferRT)))
	{
		SDL_Log("Failed to get backbuffer from swapchain.");
		return 8;
	}

	auto result = m_pD3dDevice->CreateRenderTargetView(m_pBackBufferRT, NULL, &m_pBackBufferRTView);
	if (m_pBackBufferRT) m_pBackBufferRT->Release();
	if (FAILED(result))
	{
		SDL_Log("CreateRenderTargetView failed.");
		return 9;
	}

	m_pD3dContext->OMSetRenderTargets(1, &m_pBackBufferRTView, NULL);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));
	viewport.Width = static_cast<float>(m_WindowWidth);
	viewport.Height = static_cast<float>(m_WindowHeight);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 0.f;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	m_pD3dContext->RSSetViewports(1, &viewport);

	return 0;
}

int Engine::Execute()
{
	static UINT frameDelay = 100;
	static bool isRed = false;
	static FLOAT red[4] = { 1.f, 0.f, 0.f, 1.f };
	static FLOAT blue[4] = { 0.f, 0.f, 1.f, 1.f };
	
	if (frameDelay == 0)
	{
		frameDelay = 100;

		m_pD3dContext->ClearRenderTargetView(m_pBackBufferRTView, isRed ? blue : red);
		m_pSwapChain->Present(0, 0);
		isRed = !isRed;
	}
	--frameDelay;

	return 0;
}