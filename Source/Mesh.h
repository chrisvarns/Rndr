#pragma once
#include "UniquePtr.h"
#include "SharedPtr.h"
#include <d3d11.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

struct ConstBuffer
{
	glm::mat4 mvpMatrix;
	glm::ivec4 renderMode;
};

class Mesh
{
public:
	Mesh()
		: m_NumFaces(0)
		, m_ModelMatrix(1.f)
	{}

	static SharedDeletePtr<Mesh>				LoadMesh(aiMesh* aiMesh, ID3D11Device* d3dDevice);

	glm::mat4									m_ModelMatrix;

	UniqueReleasePtr<ID3D11Buffer>				m_pVertexBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pIndexBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pNormalBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pUvBuffer;
	UniqueReleasePtr<ID3D11Buffer>				m_pConstantBuffer;

	uint32_t									m_NumFaces;
};