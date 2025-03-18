#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>

#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"


class FrustumRenderer {
public: 
	FrustumRenderer(ID3D11Device* device, DirectX::XMFLOAT3* corners);
	~FrustumRenderer();
	void Initialize(ID3D11Device* device, DirectX::XMFLOAT3* corners);
	void Render(ID3D11DeviceContext* context, ID3D11InputLayout* inputLayout, ID3D11Buffer* cameraBuffer);
	void UpdateVertexBuffer(ID3D11DeviceContext* context, DirectX::XMFLOAT3* corners);

private:
	ID3D11Buffer* vertexBuffer = nullptr;

};