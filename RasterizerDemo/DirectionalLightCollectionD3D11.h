#pragma once
#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"
#include "SamplerD3D11.h"

struct PerDirectionalLightInfo
{
	DirectX::XMFLOAT3 colour;
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	float angle = 0.0f;
	float projectionNearZ = 0.0f;
	float projectionFarZ = 0.0f;
	DirectX::XMFLOAT3 initialPosition;
};

struct DirectionalLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0; // textureResulation
	} shadowMapInfo;

	std::vector<PerDirectionalLightInfo> perLightInfo;
};

class DirectionalLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		DirectX::XMFLOAT4X4 vpMatrix;
		DirectX::XMFLOAT3 colour;
		DirectX::XMFLOAT3 direction;
		float angle = 0.0f;
		DirectX::XMFLOAT3 position;
	};

	std::vector<LightBuffer> bufferData;

	DepthBufferD3D11 shadowMaps;
	StructuredBufferD3D11 lightBuffer;
	std::vector<CameraD3D11> shadowCameras;

	// shadow maps 
	D3D11_VIEWPORT viewport;
	SamplerD3D11 sampler;


public:
	DirectionalLightCollectionD3D11() = default;
	~DirectionalLightCollectionD3D11() = default;

	void Initialize(ID3D11Device* device, const DirectionalLightData& lightInfo);

	void UpdateLightBuffers(ID3D11DeviceContext* context, DirectX::XMFLOAT3 lightDir);

	UINT GetNrOfLights() const;
	ID3D11DepthStencilView* GetShadowMapDSV(UINT lightIndex) const;
	ID3D11ShaderResourceView* GetShadowMapsSRV() const;
	ID3D11ShaderResourceView* GetLightBufferSRV() const;
	ID3D11Buffer* GetLightCameraConstantBuffer(UINT lightIndex) const;
	D3D11_VIEWPORT GetViewport() const;


	void BindDirectionallights(ID3D11DeviceContext* context);
	void UnBindDirectionalLights(ID3D11DeviceContext* context);
};