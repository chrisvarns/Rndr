#include <vector>
#include <memory>
#include <sdl/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

#include "RHI/RHI.h"

SharedDeletePtr<Mesh> Mesh::LoadMesh(aiMesh* aimesh, RHI::RHI* rhi)
{
	SharedDeletePtr<Mesh> mesh(new Mesh());
	mesh->m_NumFaces = aimesh->mNumFaces;
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

	//TODO Somehow cope with scenes of varying scale
	for (uint32_t i = 0; i < aimesh->mNumVertices; ++i)
	{
		aimesh->mVertices[i] /= 1000;
	}

    // Position
    mesh->m_pPositionBuffer = rhi->CreateVertexBuffer(aimesh->mVertices, aimesh->mNumVertices);
    assert(mesh->m_pPositionBuffer);

    // Normal
    mesh->m_pNormalBuffer = rhi->CreateVertexBuffer(aimesh->mNormals, aimesh->mNumVertices);
    assert(mesh->m_pNormalBuffer);

    // UV
	assert(aimesh->GetNumUVChannels() == 1);
	assert(aimesh->mTextureCoords[0]);
    mesh->m_pUvBuffer = rhi->CreateVertexBuffer(aimesh->mTextureCoords[0], aimesh->mNumVertices);
    assert(mesh->m_pUvBuffer);

    // Indices
    mesh->m_pIndexBuffer = rhi->CreateIndexBuffer(indices);

    // ConstantBuffer
    mesh->m_pConstantBuffer = rhi->CreateConstantBuffer();

	return mesh;
}