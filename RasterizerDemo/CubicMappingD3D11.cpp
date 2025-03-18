#include "CubicMappingD3D11.h"

CubicMappingD3D11::CubicMappingD3D11(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, DirectX::XMFLOAT3 initialPosition,
	DXGI_FORMAT format, bool hasSRV)
{
	Initialize(device, context, width, height, initialPosition, format, hasSRV);
}

CubicMappingD3D11::~CubicMappingD3D11()
{
	if (texture)
	{
		texture->Release();
	}

	for (int i = 0; i < 6; i++)
	{
		if (rtv[i])
		{
			rtv[i]->Release();
		}
		if (uav[i])
		{
			uav[i]->Release();
		}
	}
	

	if (srv)
	{
		srv->Release();
	}


}

void CubicMappingD3D11::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, DirectX::XMFLOAT3 initialPosition,
	DXGI_FORMAT format, bool hasSRV)
{

	// width = 1024

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &texture);

	if (FAILED(hr))
	{
		throw std::runtime_error("Could not create texture cube");
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < 6; i++)
	{
		// Create RTV for each cube face
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateRenderTargetView(texture, &rtvDesc, &rtv[i]);

		if (FAILED(hr))
		{
			throw std::runtime_error("Could not create texture cube rtv");
		}

		// Create UAV for each cube face
		uavDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateUnorderedAccessView(texture, &uavDesc, &uav[i]);
		if (FAILED(hr))
		{
			throw std::runtime_error("Could not create texture cube UAV");
		}
		
	}

	if (hasSRV == true)
	{

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // This must be a CubeMap!
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		device->CreateShaderResourceView(texture, &srvDesc, &srv);

		if (FAILED(hr))
		{
			throw std::runtime_error("Could not create texture cube srv");
		}
	}

	// Camera Initialization 
	ProjectionInfo projectionInfo;
	projectionInfo.fovAngleY = DirectX::XM_PIDIV2; // 90 degree
	projectionInfo.aspectRatio = 1.0f; // width == height
	projectionInfo.nearZ = 0.1f;
	projectionInfo.farZ = 100.0f;
	//float upRotations[6] = { -DirectX::XM_PIDIV2, (DirectX::XM_PIDIV2), 0.0f, (DirectX::XM_PI), DirectX::XM_PI, 0.0 }; // Rotations around local up
	//float rightRotations[6] = { 0.0f, 0.0f, (DirectX::XM_PIDIV2), -DirectX::XM_PIDIV2, 0.0f, 0.0f }; // Rotations around local right vector


	// around X
	float upRotations[6] = { 0.0f, 0.0, (DirectX::XM_PIDIV2), -(DirectX::XM_PIDIV2), 0.0f,0.0f }; // Rotations around local up
	// around Y
	float rightRotations[6] = { (DirectX::XM_PIDIV2), -(DirectX::XM_PIDIV2),0.0f, 0.0f, 0.0f, -DirectX::XM_PI}; // Rotations around local right vector

	for (int i = 0; i < 6; ++i)
	{
		cameras[i].Initialize(device, projectionInfo, initialPosition); // Creates an internal buffer with necessary information about the camera
		cameras[i].RotateUp(rightRotations[i]);
		cameras[i].RotateRight(upRotations[i]);
		cameras[i].UpdateInternalConstantBuffer(context);
	}

	// 
	depthBuffer.Initialize(device, width, height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	std::optional<std::array<float, 4>> borderColor = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
	sampler.Initialize(device, D3D11_TEXTURE_ADDRESS_BORDER, borderColor);

	pixelShader.Initialize(device,
		ShaderType::PIXEL_SHADER, L"..\\x64\\Debug\\PixelShaderCubeMapping.cso");;
	vertexShader.Initialize(device,
		ShaderType::VERTEX_SHADER, L"..\\x64\\Debug\\VertexShaderCubeMapping.cso");
}

void CubicMappingD3D11::RenderReflectiveObject(ID3D11DeviceContext* context, DeferredRenderer& deferredRenderer,
	SpotLightCollectionD3D11& spotLights, DirectionalLightCollectionD3D11& directionalLights, 
	SceneD3D11& Scene, ID3D11Buffer* tessellationConstantBuffer,
	ID3D11ShaderResourceView* DisplacementMapSRV, ID3D11SamplerState* DisplacementMapSamplerState,
	ID3D11Buffer* vertexConstantBufferInterface, ID3D11ShaderResourceView* IMGuiSRV, ID3D11ComputeShader* compute, 
	ID3D11InputLayout* inputLayout, DirectX::BoundingFrustum& cameraWorldFrustum)
{
	ID3D11DepthStencilView* depth = depthBuffer.GetDSV(0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(0, 1, &nullSRV);  // Make sure that the texture cube is not still bound since before as a read

	for (int i = 0; i < 6; ++i)
	{
		context->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

		float clearColour[4] = { 0.169, 0.18, 0.98, 1 };
		
		context->ClearRenderTargetView(rtv[i], clearColour);

		context->IASetInputLayout(inputLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

		ID3D11Buffer* cameraBuffer = cameras[i].GetConstantBuffer();

		deferredRenderer.BeginGeometryPass(context, depth);

		context->HSSetConstantBuffers(0, 1, &tessellationConstantBuffer);
		context->DSSetShaderResources(0, 1, &DisplacementMapSRV);
		context->DSSetSamplers(0, 1, &DisplacementMapSamplerState);
		context->VSSetConstantBuffers(0, 1, &cameraBuffer);
		context->DSSetConstantBuffers(0, 1, &cameraBuffer);
		context->RSSetViewports(1, &viewport);

		Scene.Render(context, vertexConstantBufferInterface, cameraWorldFrustum, true);

		deferredRenderer.EndGeometryPass(context);


		// === computer pass ===
		spotLights.BindSpotlights(context);
		directionalLights.BindDirectionallights(context);

		context->CSSetShaderResources(5, 1, &IMGuiSRV); // Bind SRV to slot 5

		deferredRenderer.RenderLightingPass(context, compute, uav[i]);

		spotLights.UnBindSportLights(context);
		directionalLights.UnBindDirectionalLights(context);
		
			
		//context->OMSetRenderTargets(1, &rtv[i], depth);
	}

	ID3D11RenderTargetView* nullRTV = nullptr;
	context->OMSetRenderTargets(1, &nullRTV, nullptr); // Make sure that the texture cube is not bound for writing later
}

void CubicMappingD3D11::bindCubeMapping(ID3D11DeviceContext* context, SceneObjectD3D11& object, ID3D11Buffer* cameraMainConstantBuffer,
	ID3D11Buffer* cameraPosBuffer, ID3D11Buffer* vertexConstantBufferInterface) {
	
	
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->PSSetShaderResources(0, 1, nullSRV);
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->CSSetShader(nullptr, nullptr, 0);
	vertexShader.BindShader(context);
	pixelShader.BindShader(context);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetConstantBuffers(0, 1, &cameraMainConstantBuffer);
	context->VSSetConstantBuffers(2, 1, &vertexConstantBufferInterface);

	context->PSSetConstantBuffers(0, 1, &cameraPosBuffer);
	context->PSSetShaderResources(0, 1, &srv); // Bind Cube Map

	object.Render(context, true);


	ID3D11ShaderResourceView* srvNullptr = nullptr;
	context->PSSetShaderResources(0, 1, &srvNullptr);
	
}

ID3D11Buffer* CubicMappingD3D11::GetCameraConstantBuffer(int index)
{
	return cameras[index].GetConstantBuffer();
}
