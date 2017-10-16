#pragma once
#include <d3d11.h>
#include <vector>

#include "RHI\RHI.h"
#include "UniquePtr.h"

class Window;

namespace RHI {
namespace D3D11
{

class D3D11RHI : public RHI
{
public:
    ~D3D11RHI();

    virtual bool InitRHI(const Window& window) override;
    virtual void HandleWindowResize(uint32_t windowWidth, uint32_t windowHeight) override;

    virtual RHIVertexBufferHandle CreateVertexBuffer(const aiVector3D* data, uint32_t numVertices) override;
    virtual RHIIndexBufferHandle CreateIndexBuffer(const std::vector<IndexType>& indices) override;
    virtual RHIConstantBufferHandle CreateConstantBuffer() override;
    virtual void LoadVertexShader() override;
    virtual void LoadPixelShader() override;

    virtual bool UpdateConstantBuffer(RHIConstantBufferHandle cbHandle, const ConstantBufferData& cb) override;
    
    virtual void ClearBackBuffer(const std::array<float, 4>& clearColor) override;

    virtual void SetVertexShader() override;
    virtual void SetPixelShader() override;

    virtual void DrawMesh(const Mesh& mesh) override;

    virtual void Present() override;

    virtual void Release() override;

    ID3D11Device* GetDevice() const { return m_pD3dDevice.get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return m_pD3dContext.get(); }

private:
    void RecreateBackBufferRTAndView(uint32_t windowWidth, uint32_t windowHeight);

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

    /* The RHI ensures these objects get cleaned up upon destruction, or upon a call to Release() */
    std::vector<UniqueReleasePtr<ID3D11Buffer>> m_ReleasableObjects;
};

}
}