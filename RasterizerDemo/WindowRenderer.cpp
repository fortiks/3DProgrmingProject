#include "WindowRenderer.h"

WindowRenderer::~WindowRenderer()
{
    swapChain->Release();
    graphics = nullptr;
	wireframeState->Release();

}

bool WindowRenderer::Initialize(Graphics* graphics)
{
	this->graphics = graphics;
	if (!CreateInterfaces())
	{
		throw std::runtime_error("Failed to create the swap chain.");
	}

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME; // Change to WIREFRAME mode
	rasterDesc.CullMode = D3D11_CULL_NONE;      // Disable backface culling
	rasterDesc.FrontCounterClockwise = false;

	ID3D11Device* device = graphics->GetDevice();
	ID3D11DeviceContext* context = graphics->GetContext();;

	HRESULT hr = device->CreateRasterizerState(&rasterDesc, &wireframeState);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create Rasterizer State.");
	}

	renderTargetView.InitializeBackBuffer(device, this->swapChain);
	
	depthStencilView.Initialize(device, this->graphics->GetWidth(), this->graphics->GetHeight());

	

	
	return true;

}

void WindowRenderer::Render(ID3D11InputLayout* inputLayout,
	ID3D11Buffer* cameraBuffer)
{
	graphics->GetContext()->VSSetConstantBuffers(0, 1, &cameraBuffer);
	graphics->GetContext()->DSSetConstantBuffers(0, 1, &cameraBuffer);
	graphics->Render();
	//
	graphics->GetContext()->IASetInputLayout(inputLayout);
	graphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	

}



void WindowRenderer::SetRenderTarget() const
{
	ID3D11RenderTargetView* renderTarget = renderTargetView.GetRTV();
	ID3D11DepthStencilView* depth = depthStencilView.GetDSV(0);
	graphics->GetContext()->OMSetRenderTargets(1, &renderTarget, depthStencilView.GetDSV(0));
	
}

void WindowRenderer::ClearRenderTargets() const
{
	ID3D11RenderTargetView* renderTarget = renderTargetView.GetRTV();
	ID3D11DepthStencilView* depth = depthStencilView.GetDSV(0);
	graphics->GetContext()->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	graphics->GetContext()->OMSetRenderTargets(1, &renderTarget, depthStencilView.GetDSV(0));
	float clearColour[4] = { 0.169, 0.18, 0.98, 1 };

	graphics->GetContext()->ClearRenderTargetView(renderTarget, clearColour);
	graphics->GetContext()->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	
	ID3D11RenderTargetView* nullRTV = nullptr;
	graphics->GetContext()->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void WindowRenderer::SetSwapChain() const
{
	swapChain->Present(1, 0); // synchroinze with the vertical refresh rate of the mointor 
}

void WindowRenderer::RenderWithWires()
{
	graphics->GetContext()->RSSetState(wireframeState);
}


bool WindowRenderer::CreateInterfaces()
{
	UINT flags = 0;
#ifdef _DEBUG
	if (_DEBUG)
		flags = D3D11_CREATE_DEVICE_DEBUG;  // Enable debug mode
#endif

	ID3D11Device* device = graphics->GetDevice();
	ID3D11DeviceContext* context = graphics->GetContext();


	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferDesc.Width = graphics->GetWidth();
	swapChainDesc.BufferDesc.Height = graphics->GetHeight();
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	
	// Default
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = graphics->GetWindow();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//swapChainDesc.Flags = DXGI_USAGE_UNORDERED_ACCESS;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, 
			D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context);

	graphics->SetDevice(device);
	graphics->SetContext(context);

	device = nullptr;
	context = nullptr;

	return !(FAILED(hr));
    
}


