#pragma once
#include <stdexcept>
#include <d3d11_4.h>
#include "CameraD3D11.h"
#include "SamplerD3D11.h"
#include "DepthBufferD3D11.h"

#include "SpotLightCollectionD3D11.h"
#include "DeferredRendering.h"
#include "SceneD3D11.h"
#include "DirectionalLightCollectionD3D11.h"

enum TEXTURE_CUBE_FACE_INDEX
{
	POSITIVE_X = 0,
	NEGATIVE_X = 1,
	POSITIVE_Y = 2,
	NEGATIVE_Y = 3,
	POSITIVE_Z = 4,
	NEGATIVE_Z = 5
};

struct cameraPosBufferData
{
	DirectX::XMFLOAT3 cameraPosition;
	float padding;
};


class CubicMappingD3D11
{
private:
	ID3D11Texture2D* texture = nullptr;
	ID3D11RenderTargetView* rtv[6];
	ID3D11UnorderedAccessView* uav[6];  // UAVs for deferred G-Buffer
	ID3D11ShaderResourceView* srv;
	ShaderD3D11 pixelShader;
	ShaderD3D11 vertexShader;

	CameraD3D11 cameras[6];
	D3D11_VIEWPORT viewport;
	DepthBufferD3D11 depthBuffer;


	TEXTURE_CUBE_FACE_INDEX faceIndex;

	SamplerD3D11 sampler;

public:
	CubicMappingD3D11() = default;
	CubicMappingD3D11(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, DirectX::XMFLOAT3 initialPosition,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool hasSRV = true);
	~CubicMappingD3D11();

	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, DirectX::XMFLOAT3 initialPosition,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool hasSRV = true);
	
	void RenderReflectiveObject(ID3D11DeviceContext* context, DeferredRenderer& deferredRenderer, 
		SpotLightCollectionD3D11& spotLight,
		DirectionalLightCollectionD3D11& directionalLights, SceneD3D11& Scene, ID3D11Buffer* tessellationConstantBuffer,
		ID3D11ShaderResourceView* DisplacementMapSRV, ID3D11SamplerState* DisplacementMapSamplerState, 
		ID3D11Buffer* vertexConstantBufferInterface, ID3D11ShaderResourceView* IMGuiSRV, ID3D11ComputeShader* compute, 
		ID3D11InputLayout* inputLayout, DirectX::BoundingFrustum& cameraWorldFrustum);
	
	void bindCubeMapping(ID3D11DeviceContext* context, SceneObjectD3D11& object, ID3D11Buffer* cameraMainConstantBuffer, 
		ID3D11Buffer* cameraPosBuffer, ID3D11Buffer* vertexConstantBufferInterface);

	ID3D11Buffer* GetCameraConstantBuffer(int index);
};