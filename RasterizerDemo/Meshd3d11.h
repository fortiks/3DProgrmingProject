#pragma once

#include <vector>
#include <directxmath.h>
#include <unordered_map> 

#include <d3d11_4.h>

#include "SubMeshD3D11.h"
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "SamplerD3D11.h"
#include "ConstantBufferD3D11.h"


struct MeshVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 color;

};

struct SubMeshInfo
{
	size_t startIndexValue;
	size_t nrOfIndicesInSubMesh;
	//ID3D11ShaderResourceView* ambientTextureSRV;
	ID3D11ShaderResourceView* diffuseTextureSRV;
	//ID3D11ShaderResourceView* specularTextureSRV;
};

struct MeshData
{
	struct VertexInfo
	{
		size_t sizeOfVertex;
		size_t nrOfVerticesInBuffer;
		void* vertexData;
	} vertexInfo;

	struct IndexInfo
	{
		size_t nrOfIndicesInBuffer;
		uint32_t* indexData;
	} indexInfo;

	std::vector<DirectX::XMFLOAT3> vertexPositions;
	std::vector<SubMeshInfo> subMeshInfo;
};

class MeshD3D11
{
private:
	// grapichs needs
	VertexBufferD3D11 vertexBuffer;
	IndexBufferD3D11 indexBuffer;
	std::vector<SubMeshD3D11> subMeshes;
	std::vector<DirectX::XMFLOAT3> vertexPositions;

public:
	MeshD3D11() = default;
	~MeshD3D11() = default;
	MeshD3D11(const MeshD3D11& other) = delete;
	MeshD3D11(MeshD3D11&& other) = delete;


	//MeshD3D11& operator=(const MeshD3D11& other);
	MeshD3D11& operator=(MeshD3D11&& other) noexcept;

	void Initialize(ID3D11Device* device, const MeshData& meshInfo);

	void BindMeshBuffers(ID3D11DeviceContext* context) const;
	void PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const;

	size_t GetNrOfSubMeshes() const;

	std::vector<DirectX::XMFLOAT3> GetVertexPosition() const;

	ID3D11ShaderResourceView* GetDiffuseSRV(size_t subMeshIndex) const;


	// add later
	ID3D11ShaderResourceView* GetAmbientSRV(size_t subMeshIndex) const;
	
	ID3D11ShaderResourceView* GetSpecularSRV(size_t subMeshIndex) const;
};


