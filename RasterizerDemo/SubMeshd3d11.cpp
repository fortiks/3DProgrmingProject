#include "SubMeshd3d11.h"


SubMeshD3D11& SubMeshD3D11::operator=(SubMeshD3D11&& other) noexcept
{
    if (this != &other) // Prevent self-assignment
    {
        // Move primitive data
        startIndex = other.startIndex;
        nrOfIndices = other.nrOfIndices;

        // Move the pointer (transfer ownership)
        diffuseTexture = other.diffuseTexture;
        other.diffuseTexture = nullptr; // Ensure 'other' doesn't hold a dangling pointer
    }
    return *this;
}

void SubMeshD3D11::Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh, ID3D11ShaderResourceView* diffuseTextureSRV, 
        ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* specularTextureSRV, float Ns)
{

    startIndex = startIndexValue;
    nrOfIndices = nrOfIndicesInSubMesh;
    diffuseTexture = diffuseTextureSRV;
    ambientTexture = ambientTextureSRV;
    specularTexture = specularTextureSRV;
    this->Ns = Ns;
}

void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext* context) const
{
    context->DrawIndexed(nrOfIndices, startIndex, 0);
}

ID3D11ShaderResourceView* SubMeshD3D11::GetAmbientSRV() const
{
    return this->ambientTexture;
}

ID3D11ShaderResourceView* SubMeshD3D11::GetDiffuseSRV() const
{
    return this->diffuseTexture;
}

ID3D11ShaderResourceView* SubMeshD3D11::GetSpecularSRV() const
{
    return this->specularTexture;
}
