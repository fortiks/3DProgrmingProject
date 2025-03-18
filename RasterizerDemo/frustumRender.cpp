#include "frustumRender.h"
#include "ConstantBufferD3D11.h"
#include <windows.h>
#include <d3d11.h>
#include <intsafe.h>

FrustumRenderer::FrustumRenderer(ID3D11Device* device, DirectX::XMFLOAT3* corners)
{
    Initialize(device, corners);
}

FrustumRenderer::~FrustumRenderer()
{
    vertexBuffer->Release();
}


void FrustumRenderer::Initialize(ID3D11Device* device, DirectX::XMFLOAT3* corners)
{
    LineVertex frustumVertices[] = {
        // Near plane (4 lines)
        { corners[0], DirectX::XMFLOAT3(1, 0, 0) }, // Red
        { corners[1], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[1], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[3], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[3], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[2], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[2], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[0], DirectX::XMFLOAT3(1, 0, 0) },

        // Far plane (4 lines)
        { corners[4], DirectX::XMFLOAT3(0, 1, 0) }, // Green
        { corners[5], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[5], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[7], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[7], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[6], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[6], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[4], DirectX::XMFLOAT3(0, 1, 0) },

        // Connecting near and far planes (4 lines)
        { corners[0], DirectX::XMFLOAT3(0, 0, 1) }, // Blue
        { corners[4], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[1], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[5], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[3], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[7], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[2], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[6], DirectX::XMFLOAT3(0, 0, 1) }
    };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // Must be DYNAMIC for mapping
    bufferDesc.ByteWidth = sizeof(LineVertex) * 24; // Correct buffer size
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Enable CPU write access
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = frustumVertices;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr))
    {
        
    }



    
}

void FrustumRenderer::Render(ID3D11DeviceContext* context, ID3D11InputLayout* m_inputLayout, ID3D11Buffer* cameraBuffer)
{
    // Bind the vertex buffer
    UINT stride = sizeof(LineVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Set the input layout and primitive topology
    context->IASetInputLayout(m_inputLayout); // Make sure to set the correct input layout
    context->VSGetConstantBuffers(0, 1, &cameraBuffer);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Draw the frustum
    context->Draw(24, 0); // 24 vertices for 12 lines
}

void FrustumRenderer::UpdateVertexBuffer(ID3D11DeviceContext* context, DirectX::XMFLOAT3* corners)
{
    LineVertex frustumVertices[] = {
        // Near plane (4 lines)
        { corners[0], DirectX::XMFLOAT3(1, 0, 0) }, // Red
        { corners[1], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[1], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[3], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[3], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[2], DirectX::XMFLOAT3(1, 0, 0) },

        { corners[2], DirectX::XMFLOAT3(1, 0, 0) },
        { corners[0], DirectX::XMFLOAT3(1, 0, 0) },

        // Far plane (4 lines)
        { corners[4], DirectX::XMFLOAT3(0, 1, 0) }, // Green
        { corners[5], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[5], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[7], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[7], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[6], DirectX::XMFLOAT3(0, 1, 0) },

        { corners[6], DirectX::XMFLOAT3(0, 1, 0) },
        { corners[4], DirectX::XMFLOAT3(0, 1, 0) },

        // Connecting near and far planes (4 lines)
        { corners[0], DirectX::XMFLOAT3(0, 0, 1) }, // Blue
        { corners[4], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[1], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[5], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[3], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[7], DirectX::XMFLOAT3(0, 0, 1) },

        { corners[2], DirectX::XMFLOAT3(0, 0, 1) },
        { corners[6], DirectX::XMFLOAT3(0, 0, 1) }
    };

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    
    memcpy(mappedResource.pData, frustumVertices, sizeof(frustumVertices));
    context->Unmap(vertexBuffer, 0);

}

