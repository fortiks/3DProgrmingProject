#pragma once
#include "RenderTargetD3D11.h"
#include "ShaderD3D11.h"

class DeferredRenderer {
public:
    DeferredRenderer(ID3D11Device* device, int width, int height);
    ~DeferredRenderer();

    void BeginGeometryPass(ID3D11DeviceContext* context, ID3D11DepthStencilView* DSV);
    void EndGeometryPass(ID3D11DeviceContext* context);

    void RenderLightingPass(ID3D11DeviceContext* context, ID3D11ComputeShader* computerShader, 
        ID3D11UnorderedAccessView* bufferUAV = nullptr);

    void InitializeBackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain);
private:
    int width, height;
    ID3D11UnorderedAccessView* backBufferUAV;    
	RenderTargetD3D11 gBuffer[3];
};
