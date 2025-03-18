#include "ParticleSystemD3D11.h"
#include <cmath>

ParticleSystemD3D11::ParticleSystemD3D11(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasUAV)
{
	Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasUAV);
}

void ParticleSystemD3D11::Initialize(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer, 
	void* bufferData, bool dynamic, bool hasUAV)
{
	
	vertexShader.Initialize(device,
		ShaderType::VERTEX_SHADER, L"..\\x64\\Debug\\VertexShaderParticleSystems.cso");


	computeShader.Initialize(device,
		ShaderType::COMPUTE_SHADER, L"..\\x64\\Debug\\ComputeShaderParticleSystem.cso");


	pixelShader.Initialize(device,
		ShaderType::PIXEL_SHADER, L"..\\x64\\Debug\\PixelShaderParticleSystem.cso");

	geometryShader.Initialize(device,
		ShaderType::GEOMETRY_SHADER, L"..\\x64\\Debug\\GeometryShaderParticleSystem.cso");

	particleBuffer.Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasUAV);
}

void ParticleSystemD3D11::bindSystem(ID3D11DeviceContext* context)
{
	ID3D11UnorderedAccessView* uav = particleBuffer.GetUAV();
	computeShader.BindShader(context);
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	context->Dispatch(std::ceil(particleBuffer.GetNrOfElements() / 32.0f), 1, 1);


	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	context->CSSetShader(nullptr, nullptr, 0);
}

void ParticleSystemD3D11::drawSystem(ID3D11DeviceContext* context, ID3D11Buffer* cameraConstantBuffer)
{
	context->IASetInputLayout(nullptr);

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	vertexShader.BindShader(context);

	ID3D11ShaderResourceView* srv = particleBuffer.GetSRV();
	context->VSSetShaderResources(0, 1, &srv);
	context->VSSetConstantBuffers(0, 1, &cameraConstantBuffer);

	geometryShader.BindShader(context);
	context->GSSetConstantBuffers(0, 1, &cameraConstantBuffer);

	pixelShader.BindShader(context);

	context->Draw(particleBuffer.GetNrOfElements(), 0);



	context->GSSetShader(nullptr, nullptr, 0);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->VSSetShaderResources(0, 1, &nullSRV);
}
