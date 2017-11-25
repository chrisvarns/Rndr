#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <assimp\vector3.h>

class Window;
class Mesh;
class CPUTexture;

namespace RHI {

struct ConstantBufferData
{
    glm::mat4 mvpMatrix;
    glm::ivec4 renderMode;
};

typedef uint16_t IndexType;

typedef void* RHIConstantBufferHandle;
typedef void* RHIVertexBufferHandle;
typedef void* RHIIndexBufferHandle;
typedef void* RHITexture2DHandle;

class RHI
{
public:
    virtual ~RHI() {};

    virtual bool InitRHI(const Window& window) = 0;
    virtual void HandleWindowResize(uint32_t windowWidth, uint32_t windowHeight) = 0;

    virtual RHIVertexBufferHandle CreateVertexBuffer(const aiVector3D* data, uint32_t numVertices) = 0;
    virtual RHIIndexBufferHandle CreateIndexBuffer(const std::vector<IndexType>& indices) = 0;
    virtual RHIConstantBufferHandle CreateConstantBuffer() = 0;
    virtual RHITexture2DHandle CreateTexture2D(const CPUTexture& cpuTexture) = 0;

    virtual void LoadVertexShader() = 0;
    virtual void LoadPixelShader() = 0;

    virtual bool UpdateConstantBuffer(RHIConstantBufferHandle cbHandle, const ConstantBufferData& cb) = 0;

    virtual void ClearBackBuffer(const std::array<float, 4>& clearColor) = 0;

    virtual void SetVertexShader() = 0;
    virtual void SetPixelShader() = 0;

    virtual void DrawMesh(const Mesh& mesh) = 0;
    virtual void Present() = 0;

    virtual void Release() = 0;
};

}