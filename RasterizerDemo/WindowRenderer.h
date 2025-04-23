#pragma once
#include <d3d11.h>
#include "Graphics.h"
#include "DepthBufferD3D11.h"
#include "RenderTargetD3D11.h"
#include "InputLayoutD3D11.h"
#include "IndexBufferD3D11.h"
#include "VertexBufferD3D11.h"

#include "SceneD3D11.h"

class WindowRenderer {

public:
    ~WindowRenderer();
    WindowRenderer() = default;
    bool Initialize(Graphics* graphics);
    void Render(ID3D11InputLayout* inputLayout,
        ID3D11Buffer* cameraBuffer);
    // void Resize(int width, int height); might do later if needed

    Graphics* GetGraphics() const { return graphics; };
    ID3D11DepthStencilView* GetDSV(UINT arrayIndex) const { return depthStencilView.GetDSV(arrayIndex); };

    void SetRenderTarget() const;
    void ClearRenderTargets() const;
    void SetSwapChain() const;
    IDXGISwapChain* GetSwapChain() const { return swapChain; };
    void RenderWithWires();

   
private:
    std::shared_ptr<SceneD3D11> scene;
    Graphics* graphics = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    DepthBufferD3D11 depthStencilView;

    ID3D11RasterizerState* wireframeState;
    RenderTargetD3D11 renderTargetView;

    //InputLayoutD3D11 inputLayout; // might move to shader class to create one for every vertexShader

    bool CreateInterfaces();
    
};