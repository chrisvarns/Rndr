#pragma once
#include <vector>
#include <map>
#include <d3d11.h>

#include "glm/glm.hpp"
#include "assimp/vector3.h"

#include "UniquePtr.h"

class Window;
struct CPUTexture;
class Mesh;

struct GPUTexture
{
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;
    ID3D11SamplerState* sampler;
};

struct GPURenderTarget
{
	ID3D11Texture2D* texture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;
};

struct ConstantBufferData
{
	glm::mat4 mvpMatrix;
	glm::ivec4 renderMode;
};

struct RenderTargetCreateInfo {
	int width;
	int height;
};

typedef uint16_t IndexType;

class D3D11RHI
{
public:
    // Interface stuff
    bool InitRHI(const Window& window);
    void HandleWindowResize(uint32_t windowWidth, uint32_t windowHeight);
	ID3D11Buffer* CreateVertexBuffer(const aiVector3D* data, uint32_t numVertices);
    ID3D11Buffer* CreateIndexBuffer(const std::vector<IndexType>& indices);
    ID3D11Buffer* CreateConstantBuffer();
    ID3D11Texture2D* CreateTexture2D(const CPUTexture& cpuTexture);
	ID3D11Texture2D* CreateRenderTarget(const RenderTargetCreateInfo& rtCreateInfo);
    ID3D11Texture2D*  GetDebugTexture2D();
    void LoadVertexShader();
    void LoadPixelShader();
    bool UpdateConstantBuffer(ID3D11Buffer* cbHandle, const ConstantBufferData& cb);
    void ClearBackBuffer(const std::array<float, 4>& clearColor);
    void SetVertexShader();
    void SetPixelShader();
    void DrawMesh(const Mesh& mesh);
    void Present();

    // Implementation
    ID3D11Device* GetDevice() const { return m_pD3dDevice.get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return m_pD3dContext.get(); }

private:
    void RecreateBackBufferRTAndView(uint32_t windowWidth, uint32_t windowHeight);
    void CreateDebugTexture2D();

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
    ID3D11Texture2D*                          m_DebugTexture2D;

    /* The RHI ensures these objects get cleaned up upon destruction, or upon a call to Release() */
    std::vector<UniqueReleasePtr<ID3D11DeviceChild>> m_ReleasableObjects;

	std::map<ID3D11Texture2D*, GPUTexture> m_GpuTextureMap;
	std::map<ID3D11Texture2D*, GPURenderTarget> m_GpuRenderTargetMap;
};