#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <Model.h>  // DirectXTK's Model class
#include <Effects.h> // DirectXTK's Effects class
#include <WICTextureLoader.h>

// buffers
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "ConstantBufferD3D11.h"

// Textures 
#include "SamplerD3D11.h"
#include "ShaderResourceTextureD3D11.h"

// model
#include "Meshd3d11.h"


// frustum 
#include "octree.h"


enum class RenderMode
{
    Texture, // Use UV and texture
    Color    // Use color and position
};

struct RenderType {
    int renderType;
    DirectX::XMFLOAT3 padding;
};

class SceneObjectD3D11 {

public:
    const MeshD3D11* mesh;
    DirectX::BoundingBox boundingBox;
    std::unique_ptr<ConstantBufferD3D11> constantBuffer;
    std::shared_ptr<SamplerD3D11> sampler;
    DirectX::XMMATRIX world;

    RenderMode renderMode = RenderMode::Texture;
    ConstantBufferD3D11 RenderModeConstantBuffer;

    void Render(ID3D11DeviceContext* context, bool cubeRendering = false) const;
    void updateConstBuffer(ID3D11DeviceContext* context);
    void updateBoundingBox(DirectX::XMMATRIX world);
    void Initialize(const MeshD3D11* mesh, std::shared_ptr<SamplerD3D11> sampler, ID3D11Device* device, 
        RenderMode renderMode = RenderMode::Texture);

    // Default constructor
    SceneObjectD3D11() = default;
    SceneObjectD3D11(const MeshD3D11* mesh, std::shared_ptr<SamplerD3D11> sampler, ID3D11Device* device);
    ~SceneObjectD3D11() = default;

};

class SceneD3D11
{
private:
    std::vector<std::unique_ptr<SceneObjectD3D11>> sceneObjects;
    std::vector<std::unique_ptr<SceneObjectD3D11>> movingObjects;
    OcTree<SceneObjectD3D11> octree;

public:
    SceneD3D11() = default;

    void AddObject(std::unique_ptr<SceneObjectD3D11> object);
    void AddMovingObject(std::unique_ptr<SceneObjectD3D11> object);

    SceneObjectD3D11* getObject(int ID);
    SceneObjectD3D11* getMovingObjects(int ID);
    UINT GetNrOfObjects();

    void InitializeOcTree(size_t width, size_t height);

    std::string DrawOcTree();
    
    
    void Render(ID3D11DeviceContext* context, ID3D11Buffer* IMGUIbuffer, 
        DirectX::BoundingFrustum& cameraWorldFrustum, bool CubeMapRendering = false);
    
};