#pragma once

#include <optional> // Needs C++ 17 or higher
#include <array>
#include <stdexcept>
#include <d3d11_4.h>

class SamplerD3D11
{
private:
	ID3D11SamplerState* sampler = nullptr;

public:
	SamplerD3D11() = default;
	SamplerD3D11(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode,
		std::optional<std::array<float, 4>> BorderColor = std::nullopt);
	~SamplerD3D11();

	SamplerD3D11(const SamplerD3D11& other) = delete;
	SamplerD3D11& operator=(const SamplerD3D11& other) = delete;
	SamplerD3D11(SamplerD3D11&& other) = delete;
	SamplerD3D11& operator=(SamplerD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode,
		std::optional<std::array<float, 4>> BorderColor = std::nullopt);

	ID3D11SamplerState* GetSamplerState() const;
};