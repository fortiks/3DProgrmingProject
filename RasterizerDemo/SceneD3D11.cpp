#include "SceneD3D11.h"
#include <algorithm>

SceneObjectD3D11::SceneObjectD3D11(const MeshD3D11* mesh, std::shared_ptr<SamplerD3D11> sampler, ID3D11Device* device) 
{
	Initialize(mesh, sampler, device);
}
	

void SceneObjectD3D11::updateConstBuffer(ID3D11DeviceContext* context)
{
	ConstantBufferData modelConst;
	DirectX::XMStoreFloat4x4(&modelConst.world, DirectX::XMMatrixTranspose(world));
	constantBuffer->UpdateBuffer(context, &modelConst, sizeof(modelConst));
}

void SceneObjectD3D11::updateBoundingBox(DirectX::XMMATRIX world)
{
	boundingBox.Transform(boundingBox, world);
}

void SceneObjectD3D11::Initialize(const MeshD3D11* mesh, std::shared_ptr<SamplerD3D11> sampler, ID3D11Device* device, RenderMode renderMode)
{
	this->mesh = mesh; 
	this->sampler = std::make_shared<SamplerD3D11>();
	this->sampler = sampler;
	this->renderMode = renderMode;
	this->world = DirectX::XMMatrixIdentity();
	
	// const buffer
	RenderModeConstantBuffer.Initialize(device, sizeof(RenderType), &this->renderMode);

	ConstantBufferData modelConst;
	DirectX::XMStoreFloat4x4(&modelConst.world, DirectX::XMMatrixTranspose(this->world));

	
	
	constantBuffer = std::make_unique<ConstantBufferD3D11>();

	constantBuffer->Initialize(device, sizeof(modelConst.world), &modelConst.world);


	// bounding boxes 
	std::vector<DirectX::XMFLOAT3> vertexPosition = mesh->GetVertexPosition();
	DirectX::XMFLOAT3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
	DirectX::XMFLOAT3 maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	

	for (const auto& vertex : vertexPosition) { // give the lowest x,y,z 
		minPoint.x = min(minPoint.x, vertex.x);
		minPoint.y = min(minPoint.y, vertex.y);
		minPoint.z = min(minPoint.z, vertex.z);

		maxPoint.x = max(maxPoint.x, vertex.x);
		maxPoint.y = max(maxPoint.y, vertex.y);
		maxPoint.z = max(maxPoint.z, vertex.z);
	}

	// Convert min/max points to XMVECTOR
	DirectX::XMVECTOR minVec = XMLoadFloat3(&minPoint);
	DirectX::XMVECTOR maxVec = XMLoadFloat3(&maxPoint);

	DirectX::BoundingBox::CreateFromPoints(
		boundingBox,
		minVec,
		maxVec
	);
	
}

void SceneObjectD3D11::Render(ID3D11DeviceContext* context, bool cubeRendering) const
{

	mesh->BindMeshBuffers(context);
	ID3D11SamplerState* smp = sampler->GetSamplerState(); // Bind sampler to pixel shader
	context->PSSetSamplers(0, 1, &smp);

	
	// Bind constant buffer
	ID3D11Buffer* cb = constantBuffer->GetBuffer();
	context->VSSetConstantBuffers(1, 1, &cb);

	

	if (cubeRendering == true)
	{
		size_t nrOfSubMeshes = mesh->GetNrOfSubMeshes();
		for (int i = 0; i < nrOfSubMeshes; i++)
		{
			mesh->PerformSubMeshDrawCall(context, i);

		}
	}
	else {

		ID3D11Buffer* Cb = RenderModeConstantBuffer.GetBuffer();
		context->PSSetConstantBuffers(0, 1, &Cb);
		size_t nrOfSubMeshes = mesh->GetNrOfSubMeshes();
		ID3D11ShaderResourceView* diffuseTexture = nullptr;
		for (int i = 0; i < nrOfSubMeshes; i++)
		{
			diffuseTexture = mesh->GetDiffuseSRV(i);
			context->PSSetShaderResources(0, 1, &diffuseTexture);
			mesh->PerformSubMeshDrawCall(context, i);

		}
		diffuseTexture = nullptr;

	}
	
}




void SceneD3D11::AddObject(std::unique_ptr<SceneObjectD3D11> object)
{
	sceneObjects.push_back(std::move(object)); // transfrer onwersip to scene 
}

void SceneD3D11::AddMovingObject(std::unique_ptr<SceneObjectD3D11> object)
{
	movingObjects.push_back(std::move(object));
}

SceneObjectD3D11* SceneD3D11::getObject(int ID)
{
	if (ID < 0 || ID >= static_cast<int>(sceneObjects.size())) {
		return nullptr; // Invalid ID
	}
	return sceneObjects[ID].get(); // Return raw pointer for modification
}

SceneObjectD3D11* SceneD3D11::getMovingObjects(int ID)
{
	if (ID < 0 || ID >= static_cast<int>(movingObjects.size())) {
		return nullptr; // Invalid ID
	}
	return movingObjects[ID].get(); // Return raw pointer for modification
}

void SceneD3D11::Render(ID3D11DeviceContext* context, ID3D11Buffer* IMGUIbuffer, 
	DirectX::BoundingFrustum& cameraWorldFrustum, bool CubeMapRendering)
{
	std::vector<const SceneObjectD3D11*> renderingObject = octree.CheckTree(cameraWorldFrustum);

	context->VSSetConstantBuffers(2, 1, &IMGUIbuffer); // constant vertex buffer for imgui
    
	if (CubeMapRendering)
	{
		for (auto& object : sceneObjects)
		{
			object->Render(context);
		}
	}
	else {

		std::vector<const SceneObjectD3D11*> renderingObject = octree.CheckTree(cameraWorldFrustum);
		for (const auto* object : renderingObject)
		{
			object->Render(context);
		}

	}

	for (auto& object : movingObjects)
	{
		object->Render(context);
	}


}

UINT SceneD3D11::GetNrOfObjects()
{
	return sceneObjects.size();
}

void SceneD3D11::InitializeOcTree(size_t width, size_t height)
{
	octree.Initialize(width, height);

	for (auto& object : sceneObjects) {
		octree.AddElement(object.get());
	}
}

std::string SceneD3D11::DrawOcTree()
{
	return octree.ToGraphviz();
}

