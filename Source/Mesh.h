#pragma once
#include <d3d11_1.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "UniquePtr.h"
#include "SharedPtr.h"
#include "GPUMesh.h"

class D3D11RHI;

class Mesh
{
public:
	static SharedDeletePtr<Mesh>				LoadMesh(const aiMesh& aiMesh, const aiScene& aiscene, D3D11RHI& d3dDevice);

	glm::mat4									modelMatrix;

	GPUMesh										gpuMesh;
	ID3D11Buffer*								constantBuffer;
    ID3D11Texture2D*							diffuseTexture;

	uint32_t									numFaces;
};