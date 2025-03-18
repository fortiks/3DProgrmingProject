#pragma once
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <array>
#include <string>


struct SimpleVertex
{
	float pos[3];
	float uv[2];
	float normal[3];
	float clr[3];

	SimpleVertex(const std::array<float, 3>& position, const std::array<float, 2>& UV, const std::array<float, 3>& normal,
		const std::array<float, 3>& colour = { 0,1,0 })
	{
		for (int i = 0; i < 3; ++i)
		{
			pos[i] = position[i];
			this->normal[i] = normal[i];
			clr[i] = colour[i];
		}
		uv[0] = UV[0];
		uv[1] = UV[1];
	}
};

struct PointLight
{
	float LightColor[3];  
	float LightIntensity;  
	float Lightposition[3];
	float Cameraposition[3];
	float pading[2];


	PointLight(float lightIntensity, const std::array<float, 3>& lightColor, const std::array<float, 3>& Light, const std::array<float, 3>& Camera)
	{
		for (int i = 0; i < 3; ++i)
		{
			Lightposition[i] = Light[i];
			Cameraposition[i] = Camera[i];
		}
		LightColor[0] = lightColor[0];
		LightColor[1] = lightColor[1];
		LightColor[2] = lightColor[2];
		LightIntensity = lightIntensity;
		pading[0] = 0;
		pading[1] = 0;
	}
};



struct ConstantBufferData {
	DirectX::XMFLOAT4X4 world;
};

struct LineVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color; // Optional: Color for the frustum lines
};

struct TessellationBufferData
{
	DirectX::XMFLOAT3 CameraPosition;
	float TessMax;
	float TessMin;
	float DecayFactor;
	DirectX::XMFLOAT2 padding;

};

class ConstantBufferD3D11
{
private:
	ID3D11Buffer* buffer = nullptr;
	size_t bufferSize = 0;

public:
	ConstantBufferD3D11() = default;
	ConstantBufferD3D11(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);
	~ConstantBufferD3D11();

	ConstantBufferD3D11(const ConstantBufferD3D11& other) = delete;
	ConstantBufferD3D11& operator=(const ConstantBufferD3D11& other) = delete;

	ConstantBufferD3D11(ConstantBufferD3D11&& other) noexcept; // Move constructor
	ConstantBufferD3D11& operator=(ConstantBufferD3D11&& other) noexcept; // Move assignment operator

	void Initialize(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);

	size_t GetSize() const;
	ID3D11Buffer* GetBuffer() const;

	void UpdateBuffer(ID3D11DeviceContext* context, void* data, size_t dataSize);
};