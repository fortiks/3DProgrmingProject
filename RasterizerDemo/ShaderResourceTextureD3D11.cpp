#include "ShaderResourceTextureD3D11.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void ShaderResourceTextureD3D11::LoadInTexture(image* img, const char* fname)
{
	img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0);
	if ((img->data) != NULL)
	{
		img->size = img->width * img->height * img->channels;
	}
}

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, void* textureData)
{
	Initialize(device, context, width, height, textureData);
}

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, const char* pathToTextureFile)
{
	Initialize(device, pathToTextureFile);
}

ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
	if (srv) {
		srv->Release();
	}
	if (texture) {
		texture->Release();
	}
}

ShaderResourceTextureD3D11& ShaderResourceTextureD3D11::operator=(ShaderResourceTextureD3D11&& other) noexcept
{
	if (this != &other) {
		// Release the current SRV
		if (srv) {
			srv->Release();
		}
		if (texture)
		{
			texture->Release();
		}

		// Transfer ownership of the SRV
		srv = other.srv;
		texture = other.texture;
		other.texture = nullptr;
		other.srv = nullptr;
	}
	return *this;
}

ShaderResourceTextureD3D11& ShaderResourceTextureD3D11::operator=(const ShaderResourceTextureD3D11& other)
{
	if (this != &other) {
		// Release the current SRV
		if (srv) {
			srv->Release();
		}
		if (texture)
		{
			texture->Release();
		}

		// Transfer ownership of the SRV
		srv = other.srv;
		if (srv) {
			srv->AddRef();
		}
		texture = other.texture;
		if (texture)
		{
			texture->AddRef();
		}
		
	}
	return *this;
}




void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height, void* textureData)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Assuming RGBA format
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;

	// create texture
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &texture))) {
		throw std::runtime_error("Failed to create texture.");
	}
	// Copy texture data
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, textureData, width * height * 4); // Assuming RGBA data size
	context->Unmap(texture, 0);

	// Create SRV (Shader Resource View)
	if (FAILED(device->CreateShaderResourceView(texture, nullptr, &srv))) {
		throw std::runtime_error("Failed to create shader resource view.");
	}
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, const char* pathToTextureFile)
{
	image img;
	LoadInTexture(&img, pathToTextureFile);
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = img.width;
	textureDesc.Height = img.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &img.data[0];
	data.SysMemPitch = img.width * 4;
	data.SysMemSlicePitch = 0;

	// Create texture
	if (FAILED(device->CreateTexture2D(&textureDesc, &data, &texture)))
	{
		throw std::runtime_error("Failed to create texture!");
	}

	// Create SRV (Shader Resource View)
	if (FAILED(device->CreateShaderResourceView(texture, nullptr, &srv))) {
		throw std::runtime_error("Failed to create ShaderResourceView!");
	}
}

ID3D11ShaderResourceView* ShaderResourceTextureD3D11::GetSRV() const
{
	return srv;
}
