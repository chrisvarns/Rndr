#pragma once
#include <d3d11.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "RHI/RHI.h"
#include "UniquePtr.h"
#include "SharedPtr.h"

namespace RHI {
class RHI;
}

class Mesh
{
public:
	static SharedDeletePtr<Mesh>				LoadMesh(const aiMesh& aiMesh, const aiScene& aiscene, RHI::RHI& d3dDevice);

	glm::mat4									m_ModelMatrix;

	RHI::RHIVertexBufferHandle       			m_pPositionBuffer;
    RHI::RHIIndexBufferHandle  				    m_pIndexBuffer;
	RHI::RHIVertexBufferHandle       			m_pNormalBuffer;
    RHI::RHIVertexBufferHandle				    m_pUvBuffer;
	RHI::RHIConstantBufferHandle				m_pConstantBuffer;

	uint32_t									m_NumFaces;
};