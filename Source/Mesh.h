#pragma once
#include <d3d11.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "UniquePtr.h"
#include "SharedPtr.h"

class D3D11RHI;

class Mesh
{
public:
	static SharedDeletePtr<Mesh>				LoadMesh(const aiMesh& aiMesh, const aiScene& aiscene, D3D11RHI& d3dDevice);

	glm::mat4									modelMatrix;

	ID3D11Buffer*       						positionBuffer;
	ID3D11Buffer*  								indexBuffer;
	ID3D11Buffer*       						normalBuffer;
	ID3D11Buffer*								uvBuffer;
	ID3D11Buffer*								constantBuffer;
    ID3D11Texture2D*							diffuseTexture;

	uint32_t									numFaces;
};