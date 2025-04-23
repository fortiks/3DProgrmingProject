#pragma once
#include <Windows.h>
#include <iostream>
#include <d3d11.h>

class Graphics {
public:
	~Graphics();
	Graphics() = default;
	Graphics(HWND hwnd, int width, int height);
	bool InitializeDirectX(HWND hwnd, int width, int height);
	void Render();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext() const;
	void SetDevice(ID3D11Device* dev) { device = dev; }
	void SetContext(ID3D11DeviceContext* ctx) { immediateContext = ctx; }
	HWND GetWindow() const;

	int GetWidth() const;
	int GetHeight() const;
private:
	// window 
	HWND window;
	int width; 
	int height;

	// directX
	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	D3D11_VIEWPORT viewport;
	

	void SetViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height);

};

// INPUT asemblerr  
// vertex shader 
// rasterizer
// PIXel shader
// output merger