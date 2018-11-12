#include <vector>
#include <memory>
#include <sdl/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

#include "Engine.h"
#include "FileUtils.h"
#include "D3D11/D3D11RHI.h"

SharedDeletePtr<Mesh> Mesh::LoadMesh(const aiMesh& aimesh, const aiScene& aiscene, D3D11RHI& rhi)
{
	SharedDeletePtr<Mesh> mesh(new Mesh());
	mesh->numFaces = aimesh.mNumFaces;
	mesh->modelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f));

	////////////////////
	// Extract index buffer
	std::vector<uint16_t> indices;
	for (uint32_t i = 0; i < aimesh.mNumFaces; ++i)
	{
		const aiFace& face = aimesh.mFaces[i];
		assert(face.mNumIndices == 3);
		assert((face.mIndices[0] & 0xFFFF) == face.mIndices[0]);
		assert((face.mIndices[1] & 0xFFFF) == face.mIndices[1]);
		assert((face.mIndices[2] & 0xFFFF) == face.mIndices[2]);
		indices.push_back(static_cast<uint16_t>(face.mIndices[0]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[1]));
		indices.push_back(static_cast<uint16_t>(face.mIndices[2]));
	}

	//TODO Somehow cope with scenes of varying scale
	for (uint32_t i = 0; i < aimesh.mNumVertices; ++i)
	{
		aimesh.mVertices[i] /= 1000;
	}

	assert(aimesh.GetNumUVChannels() == 1);
	assert(aimesh.mTextureCoords[0]);

    mesh->positionBuffer = rhi.CreateVertexBuffer(aimesh.mVertices, aimesh.mNumVertices);
    mesh->normalBuffer = rhi.CreateVertexBuffer(aimesh.mNormals, aimesh.mNumVertices);
    mesh->uvBuffer = rhi.CreateVertexBuffer(aimesh.mTextureCoords[0], aimesh.mNumVertices);
    mesh->indexBuffer = rhi.CreateIndexBuffer(indices);
    mesh->constantBuffer = rhi.CreateConstantBuffer();
    assert(mesh->positionBuffer);
    assert(mesh->normalBuffer);
    assert(mesh->uvBuffer);
    assert(mesh->indexBuffer);
    assert(mesh->constantBuffer);

    assert(aiscene.HasMaterials());
    assert(aimesh.mMaterialIndex == std::clamp<unsigned int>(aimesh.mMaterialIndex, 0, aiscene.mNumMaterials - 1));
    const auto& material = *aiscene.mMaterials[aimesh.mMaterialIndex];
    auto numDiffuseTexture = material.GetTextureCount(aiTextureType_DIFFUSE);
    if (numDiffuseTexture)
    {
        assert(numDiffuseTexture == 1);
        aiString texPath;
        auto airet = material.GetTexture(aiTextureType_DIFFUSE, 0, &texPath, NULL, NULL, NULL, NULL, NULL);
        assert(airet == aiReturn_SUCCESS);
        auto absoluteTexturePath = FileUtils::Combine(g_Engine->sceneAssetsBasePath, texPath.C_Str());
        mesh->diffuseTexture = g_Engine->textureMap.GetTexture2DFromPath(absoluteTexturePath);
    }
    else
    {
        mesh->diffuseTexture = rhi.GetDebugTexture2D();
    }

	return mesh;
}