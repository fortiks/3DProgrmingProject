#include "SamplerD3D11.h"

SamplerD3D11::SamplerD3D11(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode, std::optional<std::array<float, 4>> borderColor)
{
	Initialize(device, addressMode, borderColor);
}

SamplerD3D11::~SamplerD3D11()
{
    if (sampler)
    {
        sampler->Release();
    }
}

void SamplerD3D11::Initialize(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode, std::optional<std::array<float, 4>> borderColor)
{
    // Default sampler description
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering for all stages
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = addressMode;
    samplerDesc.AddressW = addressMode;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.BorderColor[0] = 0.0f; // Default to black border color
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Set border color if provided
    if (borderColor)
    {
        std::copy(borderColor->begin(), borderColor->end(), samplerDesc.BorderColor);
    }

    // Create the sampler state
    HRESULT hr = device->CreateSamplerState(&samplerDesc, &sampler);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create sampler state.");
    }
}

ID3D11SamplerState* SamplerD3D11::GetSamplerState() const
{
    return this->sampler;
}
