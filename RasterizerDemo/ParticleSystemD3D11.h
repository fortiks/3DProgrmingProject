#pragma once
#include "StructuredBufferD3D11.h"
#include "ShaderD3D11.h"
#include <stdexcept>
#include <d3d11_4.h>
#include <DirectXMath.h>


#define MAX_PARTICLES 10000  

struct FireParticle
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;  // Moves upward			
	float lifetime;   // Shorter lifespan than smoke
	float size;
	DirectX::XMFLOAT4 color;  
};

struct FireParticleCameraData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT3 position;
	float padding;
	DirectX::XMFLOAT4 padding2;
	
};

class ParticleSystemD3D11
{
private:
	StructuredBufferD3D11 particleBuffer; 
	ShaderD3D11 vertexShader;
	ShaderD3D11 computeShader;
	ShaderD3D11 pixelShader;
	ShaderD3D11 geometryShader;

public:
	ParticleSystemD3D11() = default;
	ParticleSystemD3D11(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true, bool hasUAV = false);
	~ParticleSystemD3D11() = default;

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true, bool hasUAV = false);

	void bindSystem(ID3D11DeviceContext* context);
	void drawSystem(ID3D11DeviceContext* context, ID3D11Buffer* cameraConstantBuffer);
};