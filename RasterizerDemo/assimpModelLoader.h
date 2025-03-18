#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include "SceneD3D11.h"
#include "ConstantBufferD3D11.h"


class DirectXModel{
public:
    DirectXModel(ID3D11Device * device, const std::string & modelPath);
    ~DirectXModel();

    void Render(ID3D11DeviceContext* deviceContext);

private:
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
    void LoadTextures(const aiScene* scene, const std::string& modelPath);

    std::vector<SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<ID3D11ShaderResourceView*> textures;

    SceneObjectD3D11 sceneObject; // Your SceneObjectD3D11 instance
};