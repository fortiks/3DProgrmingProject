#include "RenderTargetD3D11.h"


RenderTargetD3D11::~RenderTargetD3D11()
{
    if (srv)
    {
        srv->Release();
        srv = nullptr;
    }

    if (rtv)
    {
        rtv->Release();
        rtv = nullptr;
    }

    if (texture)
    {
        texture->Release();
        texture = nullptr;
    }
}

void RenderTargetD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, bool hasSRV)
{
    if (!device)
    {
        throw std::runtime_error("Device cannot be null.");
    }

    // Create the texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

    if (hasSRV)
    {
        textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &texture);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create texture for render target.");
    }

    // Create the render target view (RTV)
    hr = device->CreateRenderTargetView(texture, nullptr, &rtv);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create render target view.");
    }

    // Create the shader resource view (SRV) if requested
    if (hasSRV)
    {
        hr = device->CreateShaderResourceView(texture, nullptr, &srv);
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create shader resource view.");
        }
    }
}

void RenderTargetD3D11::InitializeBackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain)
{
    // get the address of the back buffer
    //ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture))))
    {
        throw std::runtime_error("Failed to get back buffer!");
    }

    // use the back buffer address to create the render target
    // null as description to base it on the backbuffers values
    HRESULT hr = device->CreateRenderTargetView(texture, NULL, &rtv);

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create RenderTargetView!");
    }
    //backBuffer->Release();

}

ID3D11RenderTargetView* RenderTargetD3D11::GetRTV() const
{
    return rtv;
}

ID3D11ShaderResourceView* RenderTargetD3D11::GetSRV() const
{
    return srv;
}

