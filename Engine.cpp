#include "Engine.h"
#include <SDL.h>
#include <string>
#include <sstream>
#include <memory>
#include <d3d11.h>
#include <d3dcompiler.h>

using namespace std;
using namespace DirectX;

VertexDesc Triangle[] = {
	{ XMFLOAT3(0.5f, 0.5f, 0.5f)/*, XMFLOAT4(1.f, 0.f, 0.f, 1.f) */},
	{ XMFLOAT3(0.5f, -0.5f, 0.5f)/*, XMFLOAT4(0.f, 1.f, 0.f, 1.f) */},
	{ XMFLOAT3(-0.5f, -0.5f, 0.5f)/*, XMFLOAT4(0.f, 0.f, 1.f, 1.f) */}
};

Engine::Engine(int argc, char** argv)
	: m_NumCmdLineArgs(argc)
	, m_CmdLineArgs(argv)
	, m_WindowWidth(1280)
	, m_WindowHeight(720)
{
}

Engine::~Engine()
{
	Release();
}

int Engine::Release()
{
	return 0;
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

		if (cmd == "xres")
		{
			m_WindowWidth = stoi(arg);
		}
		else if (cmd == "yres")
		{
			m_WindowHeight = stoi(arg);
		}
		else if (cmd == "shaderDir")
		{
			m_ShaderDir = arg;
		}
		else return 1;
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

	m_pSdlWindow.reset(SDL_CreateWindow(
		"Rndr",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_WindowWidth, m_WindowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

	if (!m_pSdlWindow) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		return 3;
	}

	ZeroMemory(&m_SdlWindowWMInfo, sizeof(SDL_SysWMinfo));
	if (!SDL_GetWindowWMInfo(m_pSdlWindow.get(), &m_SdlWindowWMInfo))
	{
		SDL_Log("SDL_GetWindowWMInfo failed.");
		return 4;
	}

	UniqueReleasePtr<IDXGIFactory1> pFactory;
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)pFactory.GetRef())))
	{
		SDL_Log("CreateDXGIFactory1 failed.");
		return 5;
	}

	IDXGIAdapter1* pTmpAdapter = NULL;
	std::vector<UniqueReleasePtr<IDXGIAdapter1> > adapters;
	UINT itr = 0;
	while (pFactory->EnumAdapters1(itr++, &pTmpAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapters.push_back(UniqueReleasePtr<IDXGIAdapter1>(pTmpAdapter));
	}
	if (adapters.size() > 1)
	{
		// Find the NVidia one
		for (UINT i = 0; i < adapters.size(); ++i)
		{
			DXGI_ADAPTER_DESC1 adapterDesc;
			adapters[i]->GetDesc1(&adapterDesc);

			// Get the nvidia adapter
			if (adapterDesc.VendorId == 4318)
			{
				m_pAdapter = std::move(adapters[i]);
				break;
			}
		}
	}
	else if (adapters.size() == 1)
	{
		// Take the first one
		m_pAdapter = std::move(adapters[0]);
	}
	if (!m_pAdapter)
	{
		SDL_Log("Failed to find device!!.");
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
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		m_pAdapter.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, creationFlags,
		featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc,
		m_pSwapChain.GetRef(), m_pD3dDevice.GetRef(), &featureLevel, m_pD3dContext.GetRef())))
	{
		SDL_Log("D3D11CreateDeviceAndSwapChain failed.");
		return 7;
	}
	
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(m_pBackBufferRT.get()), (void**)m_pBackBufferRT.GetRef())))
	{
		SDL_Log("Failed to get backbuffer from swapchain.");
		return 8;
	}

	if (FAILED(m_pD3dDevice->CreateRenderTargetView(m_pBackBufferRT.get(), NULL, m_pBackBufferRTView.GetRef())))
	{
		SDL_Log("CreateRenderTargetView failed.");
		return 9;
	}

	ID3D11RenderTargetView* backBufferRTView[] = { m_pBackBufferRTView.get() };
	m_pD3dContext->OMSetRenderTargets(1, &backBufferRTView[0], NULL);

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
	while (true)
	{
		// Handle events first
		if (HandleEvents()) break;

		// Update state
		Update(1.f);

		// Render
		Render();
	}

	return 0;
}

int Engine::HandleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			SDL_Log("SDL_QUIT");
			return 1;
			break;
		default:
			break;
		}
	}

	return 0;
}

int Engine::LoadContent()
{
	////////////////////
	// Create vertex buffer for triangle
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(Triangle);

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = Triangle;

	if (FAILED(m_pD3dDevice->CreateBuffer(&vertexDesc, &resourceData, m_pVertexBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer failed");
		return 1;
	}

	////////////////////
	// Vertex Shader
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	SDL_RWops* vsFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\SolidColourVertex.cso").c_str(), "rb");
	if (vsFile == nullptr)
	{
		SDL_Log("SDL_RWFromFile failed");
		return 2;
	}
	size_t vsDataSize = SDL_RWsize(vsFile);
	UniqueFreePtr<void> vsData;
	vsData.reset(malloc(vsDataSize));
	SDL_RWread(vsFile, vsData.get(), vsDataSize, 1);
	SDL_RWclose(vsFile);

	if (FAILED(m_pD3dDevice->CreateVertexShader(vsData.get(), vsDataSize, 0, m_pSolidColourVs.GetRef())))
	{
		SDL_Log("CreateVertexShader failed");
		return 3;
	}

	////////////////////
	// Vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	if (FAILED(m_pD3dDevice->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), vsData.get(), vsDataSize, m_pInputLayout.GetRef())))
	{
		SDL_Log("CreateInputLayout failed");
		return 4;
	}

	////////////////////
	// Pixel Shader
	SDL_RWops* psFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\SolidColourPixel.cso").c_str(), "rb");
	if (psFile == nullptr)
	{
		SDL_Log("SDL_RWFromFile failed");
		return 5;
	}
	size_t psDataSize = SDL_RWsize(psFile);
	UniqueFreePtr<void> psData;
	psData.reset(malloc(psDataSize));
	SDL_RWread(psFile, psData.get(), psDataSize, 1);
	SDL_RWclose(psFile);

	if (FAILED(m_pD3dDevice->CreatePixelShader(psData.get(), psDataSize, 0, m_pSolidColourPs.GetRef())))
	{
		SDL_Log("CreatePixelShader failed");
		return 6;
	}

	return 0;
}

int Engine::Update(FLOAT deltaTime)
{
	return 0;
}

int Engine::Render()
{
	FLOAT clearColor[4] = { 0.f, 0.f, 0.25f, 0.f };
	m_pD3dContext->ClearRenderTargetView(m_pBackBufferRTView.get(), clearColor);

	unsigned int stride = sizeof(VertexDesc);
	unsigned int offset = 0;

	m_pD3dContext->IASetInputLayout(m_pInputLayout.get());
	m_pD3dContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetRef(), &stride, &offset);
	m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3dContext->VSSetShader(m_pSolidColourVs.get(), 0, 0);
	m_pD3dContext->PSSetShader(m_pSolidColourPs.get(), 0, 0);
	m_pD3dContext->Draw(3, 0);

	m_pSwapChain->Present(0, 0);

	return 0;
}