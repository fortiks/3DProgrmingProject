#include "Meshd3d11.h"



MeshD3D11& MeshD3D11::operator=(MeshD3D11&& other) noexcept
{
	if (this != &other) // Prevent self-assignment
	{
		// Release existing resources (if necessary)
		vertexBuffer = std::move(other.vertexBuffer);
		indexBuffer = std::move(other.indexBuffer);
		subMeshes = std::move(other.subMeshes);
		vertexPositions = std::move(other.vertexPositions);

		// Leave `other` in a valid state
		other.subMeshes.clear();
	}
	return *this;
}

void MeshD3D11::Initialize(ID3D11Device* device, const MeshData& meshInfo)
{
	
	
	vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, 
			meshInfo.vertexInfo.vertexData);
	indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData);
	
	this->vertexPositions = meshInfo.vertexPositions;

	

	for (const auto& subMesh : meshInfo.subMeshInfo)
	{
		subMesh.startIndexValue;
		SubMeshD3D11 pushMesh;
		pushMesh.Initialize(subMesh.startIndexValue, subMesh.nrOfIndicesInSubMesh, subMesh.diffuseTextureSRV);
		subMeshes.push_back(pushMesh);
	}
	
}

void MeshD3D11::BindMeshBuffers(ID3D11DeviceContext* context) const
{
	// Bind vertex and index buffers
	UINT stride = vertexBuffer.GetVertexSize();;
	UINT offset = 0;
	ID3D11Buffer* vb = vertexBuffer.GetBuffer();
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

}

void MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const
{
	subMeshes[subMeshIndex].PerformDrawCall(context);
}

size_t MeshD3D11::GetNrOfSubMeshes() const
{
	return subMeshes.size();
}

std::vector<DirectX::XMFLOAT3> MeshD3D11::GetVertexPosition() const
{
	return vertexPositions;
}



ID3D11ShaderResourceView* MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
	return subMeshes[subMeshIndex].GetDiffuseSRV();
}
