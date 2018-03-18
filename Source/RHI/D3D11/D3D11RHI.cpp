#include "D3D11RHI.h"

#include <array>
#include <vector>

#include <d3d11.h>
#include <atlbase.h>

#include "sdl/SDL.h"
#include "UniquePtr.h"
#include "Engine.h"
#include "Mesh.h"
#include "FileUtils.h"

namespace RHI {
namespace D3D11 {

bool D3D11RHI::InitRHI(const Window& window)
{
#pragma region Adapter
    UniqueReleasePtr<IDXGIFactory1> pFactory;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)pFactory.GetRef())))
    {
        SDL_Log("CreateDXGIFactory1 failed.");
        return false;
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
            if (adapterDesc.VendorId == 4318 || adapterDesc.VendorId == 4098)
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
        return false;
    };

#pragma endregion

#pragma region SwapChain
    // Set up the device and swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = window.width;
    swapChainDesc.BufferDesc.Height = window.height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = window.m_SdlWindowWMInfo.info.win.window;
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
        return false;
    }
#pragma endregion

    RecreateBackBufferRTAndView(window.width, window.height);

#pragma region DepthStencilState
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
        return false;
    }

    m_pD3dContext->OMSetDepthStencilState(m_pDepthStencilState.get(), 0);
#pragma endregion

#pragma region RasterState
    // Raster state (defaults currently)
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = false;
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
        return false;
    }
    m_pD3dContext->RSSetState(m_pRasterState.get());
#pragma endregion

	CreateDebugTexture2D();

    return true;
}

void D3D11RHI::RecreateBackBufferRTAndView(uint32_t windowWidth,uint32_t windowHeight)
{
    assert(SUCCEEDED(m_pSwapChain->GetBuffer(0, __uuidof(m_pBackBufferRT.get()), (void**)m_pBackBufferRT.GetRef())));
    assert(SUCCEEDED(m_pD3dDevice->CreateRenderTargetView(m_pBackBufferRT.get(), NULL, m_pBackBufferRTView.GetRef())));

    // Set up the depth/stencil buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.Width = windowWidth;
    depthStencilDesc.Height = windowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    if (m_pDepthStencilRTView) m_pDepthStencilRTView->Release();
    if (m_pDepthStencilRT) m_pDepthStencilRT->Release();

    assert(SUCCEEDED(m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, m_pDepthStencilRT.GetRef())));
    assert(SUCCEEDED(m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilRT.get(), 0, m_pDepthStencilRTView.GetRef())));

    m_pD3dContext->OMSetRenderTargets(1, m_pBackBufferRTView.GetRef(), m_pDepthStencilRTView.get());

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = static_cast<float>(windowWidth);
    viewport.Height = static_cast<float>(windowHeight);
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    m_pD3dContext->RSSetViewports(1, &viewport);
}

void D3D11RHI::CreateDebugTexture2D()
{
    CPUTexture debugTex;
    debugTex.height = 2;
    debugTex.width = 2;
    for (int i = 0; i < debugTex.height * debugTex.width; i++)
    {
        debugTex.data.push_back(255);
        debugTex.data.push_back(0);
        debugTex.data.push_back(255);
        debugTex.data.push_back(255);
    }

    m_DebugTexture2D = CreateTexture2D(debugTex);
}

void D3D11RHI::HandleWindowResize(uint32_t windowWidth, uint32_t windowHeight)
{
    m_pBackBufferRTView->Release();
    m_pBackBufferRT->Release();
    m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    RecreateBackBufferRTAndView(windowWidth, windowHeight);
}

bool D3D11RHI::UpdateConstantBuffer(RHIConstantBufferHandle cbHandle, const ConstantBufferData& cbData)
{
    auto d3dCbHandle = reinterpret_cast<ID3D11Resource*>(cbHandle);

    D3D11_MAPPED_SUBRESOURCE cBuffer;
    if (FAILED(m_pD3dContext->Map(d3dCbHandle, 0, D3D11_MAP_WRITE_DISCARD, 0, &cBuffer)))
    {
        assert(false);
        return false;
    }

    memcpy(cBuffer.pData, &cbData, sizeof(cbData));
    m_pD3dContext->Unmap(d3dCbHandle, 0);
    return true;
}

RHIVertexBufferHandle D3D11RHI::CreateVertexBuffer(const aiVector3D* data, uint32_t numVertices)
{
    assert(data);
    assert(numVertices > 0);

    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(*data) * numVertices;

    D3D11_SUBRESOURCE_DATA dataDesc;
    ZeroMemory(&dataDesc, sizeof(dataDesc));
    dataDesc.pSysMem = data;

    ID3D11Buffer* vertexBufferHandle;
    if (FAILED(m_pD3dDevice->CreateBuffer(&bufferDesc, &dataDesc, &vertexBufferHandle)))
    {
        SDL_Log("CreateVertexBuffer failed");
        return NULL;
    }

    m_ReleasableObjects.push_back(vertexBufferHandle);
    return vertexBufferHandle;
}

RHIIndexBufferHandle D3D11RHI::CreateIndexBuffer(const std::vector<IndexType>& indices)
{
    assert(indices.size() > 0);
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(IndexType) * indices.size());

    D3D11_SUBRESOURCE_DATA dataDesc;
    ZeroMemory(&dataDesc, sizeof(dataDesc));
    dataDesc.pSysMem = indices.data();

    ID3D11Buffer* indexBufferHandle;
    if (FAILED(m_pD3dDevice->CreateBuffer(&bufferDesc, &dataDesc, &indexBufferHandle)))
    {
        SDL_Log("CreateIndexBuffer failed");
        return NULL;
    }

    m_ReleasableObjects.push_back(indexBufferHandle);
    return indexBufferHandle;
}

RHIConstantBufferHandle D3D11RHI::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC constantBufferDesc;
    ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
    constantBufferDesc.ByteWidth = sizeof(ConstantBufferData);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* constantBufferHandle;
    if (FAILED(m_pD3dDevice->CreateBuffer(&constantBufferDesc, NULL, &constantBufferHandle)))
    {
        SDL_Log("CreateConstantBuffer failed!");
        return NULL;
    }

    m_ReleasableObjects.push_back(constantBufferHandle);
    return constantBufferHandle;
}

RHITexture2DHandle D3D11RHI::CreateTexture2D(const CPUTexture& cpuTexture)
{
	auto max = glm::max(cpuTexture.width, cpuTexture.height);
	int mipLevels = 1 + glm::log2(float(max));

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = cpuTexture.width;
    textureDesc.Height = cpuTexture.height;
    textureDesc.MipLevels = mipLevels;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    GPUTexture gpuTexture;
    assert(SUCCEEDED(m_pD3dDevice->CreateTexture2D(&textureDesc, NULL, &gpuTexture.texture)));
	m_ReleasableObjects.push_back(gpuTexture.texture);

	UINT destSubresource = D3D11CalcSubresource(0, 0, textureDesc.MipLevels);
	int rowPitch = cpuTexture.width * 4;
	int depthPitch = cpuTexture.height * rowPitch;
	m_pD3dContext->UpdateSubresource(gpuTexture.texture, destSubresource, NULL, cpuTexture.data.data(), rowPitch, depthPitch);

    assert(SUCCEEDED(m_pD3dDevice->CreateShaderResourceView(gpuTexture.texture, NULL, &gpuTexture.srv)));
    m_ReleasableObjects.push_back(gpuTexture.srv);

    m_pD3dContext->GenerateMips(gpuTexture.srv);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    assert(SUCCEEDED(m_pD3dDevice->CreateSamplerState(&samplerDesc, &gpuTexture.sampler)));
    m_ReleasableObjects.push_back(gpuTexture.sampler);
    // store the gpuTexture in the map, and return the texture pointer as the handle;
    m_GpuTextureMap.insert({ gpuTexture.texture, gpuTexture });
    return gpuTexture.texture;
}

RHIRenderTargetHandle D3D11RHI::CreateRenderTarget(const RHIRenderTargetCreateInfo& rtCreateInfo)
{
	GPURenderTarget gpuRt;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.Width = rtCreateInfo.width;
	textureDesc.Height = rtCreateInfo.height;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	assert(SUCCEEDED(m_pD3dDevice->CreateTexture2D(&textureDesc, 0, &gpuRt.texture)));
	m_ReleasableObjects.push_back(gpuRt.texture);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	assert(SUCCEEDED(m_pD3dDevice->CreateRenderTargetView(gpuRt.texture, 0, &gpuRt.rtv)));
	m_ReleasableObjects.push_back(gpuRt.rtv);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	assert(SUCCEEDED(m_pD3dDevice->CreateShaderResourceView(gpuRt.texture, &srvDesc, &gpuRt.srv)));
	m_ReleasableObjects.push_back(gpuRt.srv);

	m_GpuRenderTargetMap.insert({ gpuRt.texture, gpuRt });
	return gpuRt.texture;
}

void D3D11RHI::LoadVertexShader()
{
    auto vsData = FileUtils::LoadFile("VS.cso");
    assert(SUCCEEDED(m_pD3dDevice->CreateVertexShader(vsData.data(), vsData.size(), 0, m_pSolidColourVs.GetRef())));

    ////////////////////
    // Vertex input layout
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
    {
        { "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
        { "NORMAL",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
        { "TEXCOORD",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	2,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 }
    };
    assert(SUCCEEDED(m_pD3dDevice->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), vsData.data(), vsData.size(), m_pInputLayout.GetRef())));
}

void D3D11RHI::LoadPixelShader()
{
    auto psData = FileUtils::LoadFile("PS.cso");
    assert(SUCCEEDED(m_pD3dDevice->CreatePixelShader(psData.data(), psData.size(), 0, m_pSolidColourPs.GetRef())));
}

void D3D11RHI::ClearBackBuffer(const std::array<float, 4>& clearColor)
{
    m_pD3dContext->ClearRenderTargetView(m_pBackBufferRTView.get(), clearColor.data());
    m_pD3dContext->ClearDepthStencilView(m_pDepthStencilRTView.get(), D3D11_CLEAR_DEPTH, 1.f, 0);
}

void D3D11RHI::SetVertexShader()
{
    m_pD3dContext->IASetInputLayout(m_pInputLayout.get());
    m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pD3dContext->VSSetShader(m_pSolidColourVs.get(), 0, 0);
}

void D3D11RHI::SetPixelShader()
{
    m_pD3dContext->PSSetShader(m_pSolidColourPs.get(), 0, 0);
}

RHITexture2DHandle D3D11RHI::GetDebugTexture2D()
{
    return m_DebugTexture2D;
}

void D3D11RHI::DrawMesh(const Mesh& mesh)
{
    unsigned int stride = sizeof(aiVector3D);
    unsigned int offset = 0;

    auto positionBuffer = static_cast<ID3D11Buffer*>(mesh.positionBuffer);
    auto normalBuffer = static_cast<ID3D11Buffer*>(mesh.normalBuffer);
    auto uvBuffer = static_cast<ID3D11Buffer*>(mesh.uvBuffer);
    auto indexBuffer = static_cast<ID3D11Buffer*>(mesh.indexBuffer);
    auto constantBuffer = static_cast<ID3D11Buffer*>(mesh.constantBuffer);

    m_pD3dContext->IASetVertexBuffers(0, 1, &positionBuffer, &stride, &offset);
    m_pD3dContext->IASetVertexBuffers(1, 1, &normalBuffer, &stride, &offset);
    m_pD3dContext->IASetVertexBuffers(2, 1, &uvBuffer, &stride, &offset);
    m_pD3dContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_pD3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    m_pD3dContext->PSSetConstantBuffers(0, 1, &constantBuffer);

    //Diffuse 
    auto diffuseTexture = m_GpuTextureMap.at(mesh.diffuseTexture);
    m_pD3dContext->PSSetSamplers(0, 1, &diffuseTexture.sampler);
    m_pD3dContext->PSSetShaderResources(0, 1, &diffuseTexture.srv);

    m_pD3dContext->DrawIndexed(mesh.numFaces * 3, 0, 0);
}

void D3D11RHI::Present()
{
    m_pSwapChain->Present(0, 0);
}

void D3D11RHI::Release()
{
    m_ReleasableObjects.clear();
}

D3D11RHI::~D3D11RHI()
{
    Release();
}

}
}