#pragma once
#include <vector>
#include <directxmath.h>
#include <unordered_map> 
#include <string>

#include <d3d11_4.h>
#include "MeshD3D11.h"
#include "SubMeshD3D11.h"
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "ConstantBufferD3D11.h"

struct ParsedMaterial {
	std::string mapKa;
	std::string mapKd;
	std::string mapKs;
	float shininess = 0.0f;
};


struct ParseData {
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT2> uvs;
	std::vector<DirectX::XMFLOAT3> normals;
	std::unordered_map<std::string, size_t> parsedFaces;
	std::vector<SimpleVertex> vertexData;
	std::vector<unsigned int> indexData;

	std::unordered_map<std::string, ParsedMaterial> parsedMaterials;
	

	std::string currentSubMeshMaterial;
	size_t currentSubMeshStartIndex = 0;
	std::vector<SubMeshInfo> finishedSubMeshes;

};

float GetLineFloat(const std::string& line, size_t& currentLinePos);
int GetLineInt(const std::string& line, size_t& currentLinePos);
std::string GetLineString(const std::string& line, size_t& currentLinePos);

void ReadFile(const std::string& path, std::string& toFill);

void ParsePosition(const std::string& dataSection, ParseData& data);
void ParseNormal(const std::string& dataSection, ParseData& data);
void ParseTextureCoordinates(const std::string& dataSection, ParseData& data);
void ParseFaces(const std::string& dataSection, ParseData& data);
void ParsedVertex(std::string faceElement, ParseData& data);

// matrial
void ParseMTL(const std::string director,  const std::string& dataSection, ParseData& data,
	std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, ID3D11Device* device);

// subMesh 
void PushbackCurrentSubmesh(ParseData& data, std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures);

// main functions
void ParseLine(const std::string director, const std::string& line, ParseData& data,
		std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, ID3D11Device* device);

void ParseOBJ(const std::string director, const std::string& identifier, const std::string& contents,
		std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures,
		std::unordered_map<std::string, MeshD3D11>& loadedMeshes, ID3D11Device* device);


// Mesh 
const MeshD3D11* GetMesh(const std::string director, const std::string& identifier,
	std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures,
	std::unordered_map<std::string, MeshD3D11>& loadedMeshes, ID3D11Device* device);