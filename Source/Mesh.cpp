#include "Mesh.h"
#include <vector>
#include <memory>
#include <sdl/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

SharedDeletePtr<Mesh> Mesh::LoadMesh(aiMesh* aimesh, ID3D11Device* pD3dDevice)
{
	SharedDeletePtr<Mesh> mesh(new Mesh());
	mesh->m_pNumFaces = aimesh->mNumFaces;
	mesh->m_ModelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f));

	////////////////////
	// Extract index buffer
	std::vector<uint16_t> indices;
	for (uint32_t i = 0; i < aimesh->mNumFaces; ++i)
	{
		const aiFace& face = aimesh->mFaces[i];
		assert(face.mNumIndices == 3);
		assert((face.mIndices[0] & 0xFFFF) == face.mIndices[0]);
		assert((face.mIndices[1] & 0xFFFF) == face.mIndices[1]);
		assert((face.mIndices[2] & 0xFFFF) == face.mIndices[2]);
		indices.push_back(static_cast<uint16_t>(face.mIndices[0]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[1]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[2]));
	}

	// Sponza Hack
	for (uint32_t i = 0; i < aimesh->mNumVertices; ++i)
	{
		aimesh->mVertices[i] /= 1000;
	}

	////////////////////
	// Create vertex buffer
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(*aimesh->mVertices) * aimesh->mNumVertices;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = aimesh->mVertices;

	if (FAILED(pD3dDevice->CreateBuffer(&vertexDesc, &vertexData, mesh->m_pVertexBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Vertex Pos) failed");
		return NULL;
	}

	////////////////////
	// Create normals buffer
	D3D11_BUFFER_DESC normalsDesc;
	ZeroMemory(&normalsDesc, sizeof(normalsDesc));
	normalsDesc.Usage = D3D11_USAGE_DEFAULT;
	normalsDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	normalsDesc.ByteWidth = sizeof(*aimesh->mNormals) * aimesh->mNumVertices;

	D3D11_SUBRESOURCE_DATA normalData;
	ZeroMemory(&normalData, sizeof(normalData));
	normalData.pSysMem = aimesh->mNormals;

	if (FAILED(pD3dDevice->CreateBuffer(&normalsDesc, &normalData, mesh->m_pNormalBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Vertex Normal) failed");
		return NULL;
	}

	{
	////////////////////
	// Create TexCoord buffer
	assert(aimesh->GetNumUVChannels() == 1);
	D3D11_BUFFER_DESC uvDesc;
	ZeroMemory(&uvDesc, sizeof(uvDesc));
	uvDesc.Usage = D3D11_USAGE_DEFAULT;
	uvDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	assert(aimesh->mTextureCoords[0]);
	uvDesc.ByteWidth = sizeof(**aimesh->mTextureCoords) * aimesh->mNumVertices;

	D3D11_SUBRESOURCE_DATA uvData;
	ZeroMemory(&uvData, sizeof(uvData));
	uvData.pSysMem = aimesh->mTextureCoords[0];

	if (FAILED(pD3dDevice->CreateBuffer(&uvDesc, &uvData, mesh->m_pUvBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (UV) failed");
		return NULL;
	}
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

	if (FAILED(pD3dDevice->CreateBuffer(&indicesDesc, &indicesData, mesh->m_pIndexBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Index) failed");
		return NULL;
	}

	////////////////////
	// Constant buffer
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&cBufferDesc, sizeof(cBufferDesc));
	cBufferDesc.ByteWidth = sizeof(ConstBuffer);
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(pD3dDevice->CreateBuffer(&cBufferDesc, NULL, mesh->m_pConstantBuffer.GetRef())))
	{
		SDL_Log("CreateBuffer (Constant) failed!");
		return NULL;
	}

	return mesh;
}