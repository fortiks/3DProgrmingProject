#include "Graphics.h"

Graphics::~Graphics()
{
	this->device->Release();
	this->immediateContext->Release();
}

Graphics::Graphics(HWND hwnd, int width, int height)
{
	InitializeDirectX(hwnd, width, height);
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
	this->window = hwnd;
	this->width = width;
	this->height = height;
	SetViewport(viewport, width, height);

    return true;
}

void Graphics::Render()
{
	immediateContext->RSSetViewports(1, &viewport);
}

ID3D11Device* Graphics::GetDevice()
{
	return this->device;
}

ID3D11DeviceContext* Graphics::GetContext() const
{
	return this->immediateContext;
}

HWND Graphics::GetWindow() const
{
	return window;
}

int Graphics::GetWidth() const
{
	return width;
}

int Graphics::GetHeight() const
{
	return height;
}

void Graphics::SetViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height)
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
}

