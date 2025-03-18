#include "assimpModelLoader.h"
/*
DirectXModel::DirectXModel(ID3D11Device* device, const std::string& modelPath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);


	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
	}


	ProcessNode(scene->mRootNode, scene);
	LoadTextures(scene, modelPath);

	sceneObject.vertexBuffer->Initialize(device, sizeof(SimpleVertex), vertices.size(), vertices.data());
	sceneObject.indexBuffer->Initialize(device, indices.size(), indices.data());

}

DirectXModel::~DirectXModel()
{
	for (auto& texture : textures) {
		texture->Release();
	}
}
*/