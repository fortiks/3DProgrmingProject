#include "DeferredRendering.h"

DeferredRenderer::DeferredRenderer(ID3D11Device* device, int width, int height) : width(width), height(height)
{
	
	gBuffer[0].Initialize(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
	gBuffer[1].Initialize(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
	gBuffer[2].Initialize(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
	gBuffer[3].Initialize(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
}

DeferredRenderer::~DeferredRenderer()
{
	if (backBufferUAV)
	{
		backBufferUAV->Release();
	}
}

void DeferredRenderer::BeginGeometryPass(ID3D11DeviceContext* context, ID3D11DepthStencilView* DSV)
{
	const unsigned int NR_OF_GBUFFERS = 4;
	float clearColour[4] = { 0.0, 0.0, 0.0, 0.0};

	context->ClearRenderTargetView(gBuffer[0].GetRTV(), clearColour); // clear the rtv to get inomfation again
	context->ClearRenderTargetView(gBuffer[1].GetRTV(), clearColour);
	context->ClearRenderTargetView(gBuffer[2].GetRTV(), clearColour);
	context->ClearRenderTargetView(gBuffer[3].GetRTV(), clearColour);

	ID3D11RenderTargetView* rtvArr[NR_OF_GBUFFERS];
	rtvArr[0] = gBuffer[0].GetRTV();
	rtvArr[1] = gBuffer[1].GetRTV();
	rtvArr[2] = gBuffer[2].GetRTV();
	rtvArr[3] = gBuffer[3].GetRTV();

	context->OMSetRenderTargets(NR_OF_GBUFFERS, rtvArr, DSV);
}

void DeferredRenderer::EndGeometryPass(ID3D11DeviceContext* context)
{
	// Unbind G-Buffer textures from render targets
	ID3D11RenderTargetView* nullRTV[4] = { nullptr, nullptr, nullptr, nullptr};
	context->OMSetRenderTargets(4, nullRTV, nullptr);
}

void DeferredRenderer::RenderLightingPass(ID3D11DeviceContext* context, ID3D11ComputeShader* computerShader, 
	ID3D11UnorderedAccessView* bufferUAV)
{
	const unsigned int NR_OF_GBUFFERS = 4;

	// clear backBuffer between calls
	float clearColor[4] = { 0.169f, 0.18f, 0.98f, 1.0f }; // RGBA in float (normalized 0-1)
	if (bufferUAV != nullptr)
	{
		context->ClearUnorderedAccessViewFloat(bufferUAV, clearColor);
		// Bind backbuffer UAV
		context->CSSetUnorderedAccessViews(0, 1, &bufferUAV, nullptr);
	}
	else 
	{
		context->ClearUnorderedAccessViewFloat(backBufferUAV, clearColor);
		// Bind backbuffer UAV
		context->CSSetUnorderedAccessViews(0, 1, &backBufferUAV, nullptr);
	}
	

	// Dispatch compute shader (assume 8x8 thread groups)
	ID3D11ComputeShader* CSShader = computerShader;
	context->CSSetShader(CSShader, nullptr, 0);
	
	ID3D11ShaderResourceView* gBufferSRVs[NR_OF_GBUFFERS] = {
		gBuffer[0].GetSRV(), // texture
		gBuffer[1].GetSRV(), // normal
		gBuffer[2].GetSRV(),  //pos
		gBuffer[3].GetSRV()  //texture Ambient and specular
	};

	context->CSSetShaderResources(0, NR_OF_GBUFFERS, gBufferSRVs);

	context->Dispatch(width / 8, height / 8, 1);
	//context->Flush();  // Ensure all commands are completed
	// Unbind resources
	ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr};
	context->CSSetShaderResources(0, 4, nullSRVs);
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

void DeferredRenderer::InitializeBackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain)
{
	if (backBufferUAV)
	{
		backBufferUAV->Release();
		backBufferUAV = nullptr;
	}
	// get the address of the back buffer
	ID3D11Texture2D* backBuffer = nullptr;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
	{
		throw std::runtime_error("Failed to get back buffer!");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Ensure this matches the back buffer format
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	HRESULT hr = device->CreateUnorderedAccessView(backBuffer, &uavDesc, &backBufferUAV);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create UAV!");
	}
	backBuffer->Release();
}
