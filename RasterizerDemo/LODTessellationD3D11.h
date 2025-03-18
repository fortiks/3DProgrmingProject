#pragma once
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "CameraD3D11.h"  // Assume you have a Camera class for position

using namespace DirectX;

class LODTessellationD3D11
{
private:
    
public:
    LODTessellationD3D11() = default;
    ~LODTessellationD3D11();

    void Initialize(ID3D11Device* device);

    void UpdateTessellationBuffer(ID3D11DeviceContext* context, const CameraD3D11& camera, const XMFLOAT3& patchCenter);

    void Bind(ID3D11DeviceContext* context, UINT slot = 1);

};