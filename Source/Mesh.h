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

	glm::mat4									modelMatrix;

	RHI::RHIVertexBufferHandle       			positionBuffer;
    RHI::RHIIndexBufferHandle  				    indexBuffer;
	RHI::RHIVertexBufferHandle       			normalBuffer;
    RHI::RHIVertexBufferHandle				    uvBuffer;
	RHI::RHIConstantBufferHandle				constantBuffer;
    RHI::RHITexture2DHandle                     diffuseTexture;

	uint32_t									numFaces;
};