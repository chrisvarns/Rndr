#include "Engine.h"
#include <sdl/SDL.h>
#include <string>
#include <sstream>
#include <cassert>
#include <memory>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace DirectX;

// Special behavior for ++Colors
RenderMode& operator++(RenderMode& rm) {
	rm = static_cast<RenderMode>(static_cast<int>(rm) + 1);
	if (rm == RenderMode::END_OF_LIST) rm = static_cast<RenderMode>(0);
	return rm;
}

// Special behavior for Colors++
RenderMode operator++(RenderMode& rm, int) {
	RenderMode result = rm;
	++rm;
	return result;
}

Engine::Engine(int argc, char** argv)
	: m_NumCmdLineArgs(argc)
	, m_CmdLineArgs(argv)
	, m_WindowWidth(1280)
	, m_WindowHeight(720)
	, m_ModelMatrix(1.f)
	, m_ViewMatrix(1.f)
	, m_ViewAngleV(0.f)
	, m_ViewAngleH(0.f)
	, m_ViewPos(0.f)
	, m_ProjectionMatrix(1.f)
	, m_RenderMode(RenderMode::SolidColour)
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
		else if (cmd == "mesh")
		{
			m_MeshPath = arg;
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

	m_ProjectionMatrix = glm::perspective(45.f, (float)m_WindowWidth / m_WindowHeight, 0.1f, 100.f);

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

	// Set up the depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.Width = m_WindowWidth;
	depthStencilDesc.Height = m_WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	if (FAILED(m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, m_pDepthStencilRT.GetRef())))
	{
		SDL_Log("CreateTexture2D failed.");
		return 10;
	}
	if (FAILED(m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilRT.get(), 0, m_pDepthStencilRTView.GetRef())))
	{
		SDL_Log("CreateDepthStencilView failed.");
		return 11;
	}
	m_pD3dContext->OMSetRenderTargets(1, m_pBackBufferRTView.GetRef(), m_pDepthStencilRTView.get());

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));
	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilStateDesc.StencilEnable = false;
	depthStencilStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	if (FAILED(m_pD3dDevice->CreateDepthStencilState(&depthStencilStateDesc, m_pDepthStencilState.GetRef())))
	{
		SDL_Log("CreateDepthStencilState failed.");
		return 12;
	}

	m_pD3dContext->OMSetDepthStencilState(m_pDepthStencilState.get(), 0);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));
	viewport.Width = static_cast<float>(m_WindowWidth);
	viewport.Height = static_cast<float>(m_WindowHeight);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 0.f;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	m_pD3dContext->RSSetViewports(1, &viewport);

	// Raster state, defaults except CCW winding
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.f;
	rasterDesc.DepthBiasClamp = 0.f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;
	if (FAILED(m_pD3dDevice->CreateRasterizerState(&rasterDesc, m_pRasterState.GetRef())))
	{
		SDL_Log("CreateRasterizerState failed.");
		return 13;
	}
	m_pD3dContext->RSSetState(m_pRasterState.get());

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

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_RIGHT) SDL_SetRelativeMouseMode(SDL_TRUE);
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_RIGHT) SDL_SetRelativeMouseMode(SDL_FALSE);
			break;
		case SDL_MOUSEMOTION:
			if (SDL_GetRelativeMouseMode())
			{
				m_ViewAngleH += event.motion.xrel * 0.001;
				m_ViewAngleV -= event.motion.yrel * 0.001;				
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_r:
				++m_RenderMode;
			}
			break;
		default:
			break;
		}
	}

	return 0;
}

int Engine::LoadContent()
{
	// Load the asset with assimp
	Assimp::Importer assimp;
	const aiScene* m_pScene = assimp.ReadFile(m_MeshPath,
		aiProcess_ConvertToLeftHanded	// Convert to CW for DirectX.
		| aiProcess_GenSmoothNormals
		| aiProcess_ImproveCacheLocality
	);

	aiMesh* mesh = m_pScene->mMeshes[0];
	m_pNumVerts = mesh->mNumVertices;
	m_ModelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f));

	std::vector<uint16_t> indices;
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(static_cast<uint16_t>(face.mIndices[0]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[1]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[2]));
	}

	////////////////////
	// Create vertex buffer
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(*mesh->mVertices) * mesh->mNumVertices;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = mesh->mVertices;

	if (FAILED(m_pD3dDevice->CreateBuffer(&vertexDesc, &vertexData, m_pVertexBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Vertex Pos) failed");
		return 1;
	}

	////////////////////
	// Create normals buffer
	D3D11_BUFFER_DESC normalsDesc;
	ZeroMemory(&normalsDesc, sizeof(normalsDesc));
	normalsDesc.Usage = D3D11_USAGE_DEFAULT;
	normalsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	normalsDesc.ByteWidth = sizeof(*mesh->mNormals) * mesh->mNumVertices;

	D3D11_SUBRESOURCE_DATA normalData;
	ZeroMemory(&normalData, sizeof(normalData));
	normalData.pSysMem = mesh->mNormals;

	if (FAILED(m_pD3dDevice->CreateBuffer(&normalsDesc, &normalData, m_pNormalBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Vertex Normal) failed");
		return 1;
	}

	////////////////////
	// Create Index buffer
	D3D11_BUFFER_DESC indicesDesc;
	ZeroMemory(&indicesDesc, sizeof(indicesDesc));
	indicesDesc.Usage = D3D11_USAGE_DEFAULT;
	indicesDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesDesc.ByteWidth = indices.size() * sizeof(uint16_t);

	D3D11_SUBRESOURCE_DATA indicesData;
	indicesData.pSysMem = indices.data();
	indicesData.SysMemPitch = 0;
	indicesData.SysMemSlicePitch = 0;

	if (FAILED(m_pD3dDevice->CreateBuffer(&indicesDesc, &indicesData, m_pIndexBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Index) failed");
		return 1;
	}

	////////////////////
	// Vertex Shader
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	SDL_RWops* vsFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\VS.cso").c_str(), "rb");
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
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "NORMAL",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 }
	};
	if (FAILED(m_pD3dDevice->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), vsData.get(), vsDataSize, m_pInputLayout.GetRef())))
	{
		SDL_Log("CreateInputLayout failed");
		return 4;
	}

	////////////////////
	// Pixel Shader
	SDL_RWops* psFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\PS.cso").c_str(), "rb");
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

	////////////////////
	// Constant buffer
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&cBufferDesc, sizeof(cBufferDesc));
	cBufferDesc.ByteWidth = sizeof(ConstBuffer);
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pD3dDevice->CreateBuffer(&cBufferDesc, NULL, m_pConstantBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Constant) failed!");
		return 7;
	}

	return 0;
}

int Engine::UpdateCamera(float deltaTime)
{
	// Wrap around/clamp the view angles
	if (m_ViewAngleH > glm::two_pi<float>()) m_ViewAngleH -= glm::two_pi<float>();
	else if (m_ViewAngleH < 0) m_ViewAngleH += glm::two_pi<float>();
	m_ViewAngleV = glm::clamp(m_ViewAngleV, glm::radians(-85.f), glm::radians(85.f));

	glm::vec3 viewDir(
		glm::cos(m_ViewAngleV) * glm::sin(m_ViewAngleH),
		glm::sin(m_ViewAngleV),
		glm::cos(m_ViewAngleV) * glm::cos(m_ViewAngleH)
	);

	glm::vec3 rightDir(
		glm::sin(m_ViewAngleH + glm::half_pi<float>()),
		0,
		glm::cos(m_ViewAngleH + glm::half_pi<float>())
	);

	glm::vec3 upDir = glm::cross(viewDir, rightDir);

	// If the RMB is held, we can grab WASD and update the camera pos.
	if (SDL_GetRelativeMouseMode())
	{
		const uint8_t* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_W]) m_ViewPos += viewDir * 0.001f;
		if (keyboardState[SDL_SCANCODE_S]) m_ViewPos -= viewDir * 0.001f;
		if (keyboardState[SDL_SCANCODE_A]) m_ViewPos -= rightDir * 0.001f;
		if (keyboardState[SDL_SCANCODE_D]) m_ViewPos += rightDir * 0.001f;
		if (keyboardState[SDL_SCANCODE_E]) m_ViewPos += upDir * 0.001f;
		if (keyboardState[SDL_SCANCODE_Q]) m_ViewPos -= upDir * 0.001f;
	}

	//m_ViewMatrix = glm::lookAt(m_ViewPos, m_ViewPos + viewDir, glm::vec3(0.f, 1.f, 0.f));
	m_ViewMatrix = glm::lookAt(m_ViewPos, m_ViewPos + viewDir, upDir);
	return 0;
}

int Engine::Update(float deltaTime)
{
	UpdateCamera(deltaTime);
	// Update the constant buffer...
	ConstBuffer constBuffer;
	constBuffer.mvpMatrix = m_ProjectionMatrix * m_ViewMatrix * m_ModelMatrix;
	constBuffer.renderMode = glm::ivec4(static_cast<int>(m_RenderMode));

	D3D11_MAPPED_SUBRESOURCE cBuffer;
	m_pD3dContext->Map(m_pConstantBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cBuffer);
	memcpy(cBuffer.pData, &constBuffer, sizeof(constBuffer));
	m_pD3dContext->Unmap(m_pConstantBuffer.get(), 0);

	return 0;
}

int Engine::Render()
{
	FLOAT clearColor[4] = { 0.f, 0.f, 0.25f, 1.f };
	m_pD3dContext->ClearRenderTargetView(m_pBackBufferRTView.get(), clearColor);
	m_pD3dContext->ClearDepthStencilView(m_pDepthStencilRTView.get(), D3D11_CLEAR_DEPTH, 1, 0);

	unsigned int stride = sizeof(aiVector3D);
	unsigned int offset = 0;

	m_pD3dContext->IASetInputLayout(m_pInputLayout.get());
	m_pD3dContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetRef(), &stride, &offset);
	m_pD3dContext->IASetVertexBuffers(1, 1, m_pNormalBuffer.GetRef(), &stride, &offset);
	m_pD3dContext->IASetIndexBuffer(m_pIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3dContext->VSSetShader(m_pSolidColourVs.get(), 0, 0);
	m_pD3dContext->PSSetShader(m_pSolidColourPs.get(), 0, 0);
	m_pD3dContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetRef());
	m_pD3dContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetRef());
	m_pD3dContext->Draw(m_pNumVerts, 0);

	m_pSwapChain->Present(0, 0);

	return 0;
}