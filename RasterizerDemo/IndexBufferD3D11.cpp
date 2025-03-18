#include "IndexBufferD3D11.h"

IndexBufferD3D11::IndexBufferD3D11(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = nrOfIndicesInBuffer * sizeof(DWORD);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indexData[0];
	HRESULT hr = device->CreateBuffer(&indexBufferDesc, &iinitData, &this->buffer);

}

IndexBufferD3D11::~IndexBufferD3D11()
{
	if (buffer)
	{
		buffer->Release();
	}
	
}


IndexBufferD3D11& IndexBufferD3D11::operator=(IndexBufferD3D11&& other) noexcept
{
	if (this != &other)
	{
		if (buffer)
		{
			buffer->Release();
		}

		buffer = other.buffer;
		nrOfIndices = other.nrOfIndices;

		other.buffer = nullptr;
		other.nrOfIndices = 0;
	}
	return *this;
}

void IndexBufferD3D11::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = nrOfIndicesInBuffer * sizeof(DWORD);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indexData[0];
	HRESULT hr = device->CreateBuffer(&indexBufferDesc, &iinitData, &this->buffer);
}

size_t IndexBufferD3D11::GetNrOfIndices() const
{
	return nrOfIndices;
}

ID3D11Buffer* IndexBufferD3D11::GetBuffer() const
{
	return this->buffer;
}
