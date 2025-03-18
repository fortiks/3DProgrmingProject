#include "parsedData.h"
#include "Meshd3d11.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>



float GetLineFloat(const std::string& line, size_t& currentLinePos)
{
    size_t numberStart = currentLinePos;
    int endOfRow = line.length();
    char character = line[numberStart];
    bool negative = false;
   
    while ((character != ' ') && (numberStart < endOfRow)) {
        if (character == '-') {
            negative = true;
        }
        numberStart += 1;  
        character = line[numberStart];
    }

    float extractedAndConvertedFloat = 0;
    std::string temp;
    if (negative == true) {
        temp = line.substr(currentLinePos+1, numberStart-1); // the negative in the begnning and the space/end of the row
        extractedAndConvertedFloat = std::stof(temp);
        extractedAndConvertedFloat = extractedAndConvertedFloat * -1.0f;

    }
    else {
        temp = line.substr(currentLinePos, numberStart); // the space/end of the row
        extractedAndConvertedFloat = std::stof(temp);
    }
    currentLinePos = numberStart;
    return extractedAndConvertedFloat;
}

int GetLineInt(const std::string& line, size_t& currentLinePos)
{
    size_t numberStart = currentLinePos;
    int endOfRow = line.length();
    char character = line[numberStart];
    bool negative = false;

    while (numberStart < endOfRow && character != ' ' && character != '/') {
        if (character == '-') {
            negative = true;
        }
        numberStart++;
        character = line[numberStart];
    }
    int extractedAndConvertedInt = 0;
    std::string temp;
    if (negative == true) {
        temp = line.substr(currentLinePos + 1, numberStart - 2); // the negative in the begnning and the space/end of the row
        extractedAndConvertedInt = std::stoi(temp);
        extractedAndConvertedInt = extractedAndConvertedInt * -1;

    }
    else {
        temp = line.substr(currentLinePos, numberStart - 1); // the space/end of the row
        extractedAndConvertedInt = std::stoi(temp);
    }
    currentLinePos = numberStart;
    return extractedAndConvertedInt;
}

std::string GetLineString(const std::string& line, size_t& currentLinePos)
{
    size_t numberStart = currentLinePos;
    int endOfRow = line.length();
    char character = line[numberStart];
    bool negative = false;

    while (numberStart < endOfRow && character != ' ') {
        numberStart++;
        character = line[numberStart];
    }

    std::string extractedString = line.substr(currentLinePos, numberStart-currentLinePos); // the space/end of the row
    currentLinePos = numberStart;
    return extractedString;
}

void ReadFile(const std::string& path, std::string& toFill)
{
    std::ifstream reader;
    std::string director = "";
    std::string end = path;
    reader.open(end);
    if (!reader.is_open()) {
        throw std::runtime_error("Could not open mesh data file");
    }
    reader.seekg(1, std::ios::end);
    toFill.reserve(static_cast<unsigned int>(reader.tellg()));
    reader.seekg(0, std::ios::beg);
    toFill.assign((std::istreambuf_iterator<char>(reader)),
        std::istreambuf_iterator<char>());

}

void ParsePosition(const std::string& dataSection, ParseData& data)
{
    size_t currentPos = 0; // Assuming string starts at the data section's first character
    DirectX::XMFLOAT3 toAdd;
    std::string line = dataSection;
    toAdd.x = GetLineFloat(line, currentPos);
    currentPos++; // Skip space
    toAdd.y = GetLineFloat(line, currentPos);
    currentPos++; // Skip space
    toAdd.z = GetLineFloat(line, currentPos);
    data.positions.push_back(toAdd);

}

void ParseNormal(const std::string& dataSection, ParseData& data)
{
    size_t currentPos = 0; // Assuming string starts at the data section's first character
    DirectX::XMFLOAT3 toAdd;
    toAdd.x = GetLineFloat(dataSection, currentPos);
    currentPos++; // Skip space
    toAdd.y = GetLineFloat(dataSection, currentPos);
    currentPos++; // Skip space
    toAdd.z = GetLineFloat(dataSection, currentPos);
    data.normals.push_back(toAdd);
}



void ParseTextureCoordinates(const std::string& dataSection, ParseData& data)
{
    size_t currentPos = 0; // Assuming string starts at the data section's first character
    DirectX::XMFLOAT2 toAdd;
    toAdd.x = GetLineFloat(dataSection, currentPos);
    currentPos++; // Skip space
    toAdd.y = GetLineFloat(dataSection, currentPos);

    data.uvs.push_back(toAdd);
}

void ParseFaces(const std::string& dataSection, ParseData& data)
{
    size_t currentPos = 0; // Assuming string starts at the data section's first character
    std::unordered_map<std::string, size_t> toAdd;
    std::string line = GetLineString(dataSection, currentPos);
    currentPos++; // Skip space
    if (data.parsedFaces.find(line) == data.parsedFaces.end())
    {
        data.parsedFaces[line] = data.parsedFaces.size();
        ParsedVertex(line, data);

    }
    else {
         data.indexData.push_back(data.parsedFaces[line]);
    }
    
    line = GetLineString(dataSection, currentPos);
    currentPos++; // Skip space
    if (data.parsedFaces.find(line) == data.parsedFaces.end())
    {
        data.parsedFaces[line] = data.parsedFaces.size();
        ParsedVertex(line, data);
    }
    else {
        data.indexData.push_back(data.parsedFaces[line]);
    }

    line = GetLineString(dataSection, currentPos);
    currentPos++; // Skip space
    if (data.parsedFaces.find(line) == data.parsedFaces.end())
    {
        data.parsedFaces[line] = data.parsedFaces.size();
        ParsedVertex(line, data);
    }
    else {
        data.indexData.push_back(data.parsedFaces[line]);
    }



}

// help function 
std::array<float, 3> XMFloat3ToArray(const DirectX::XMFLOAT3& xmFloat3) {
    return { xmFloat3.x, xmFloat3.y, xmFloat3.z };
}

std::array<float, 2> XMFloat2ToArray(const DirectX::XMFLOAT2& xmFloat2) {
    return { xmFloat2.x, xmFloat2.y };
}

void ParsedVertex(std::string faceElement, ParseData& data)
{
    std::istringstream lineStream(faceElement);
    std::string indexStr;

    std::getline(lineStream, indexStr, '/');
    int vertexIndex = std::stoi(indexStr) - 1; // -1 becouse faces start at 1 

    std::getline(lineStream, indexStr, '/');
    int texCoordIndex = std::stoi(indexStr) - 1;

    std::getline(lineStream, indexStr, '/');
    int normalIndex = std::stoi(indexStr) - 1;

    // vertex
    const std::array<float, 3> positionArray = XMFloat3ToArray(data.positions[vertexIndex]);
    const std::array<float, 2> UVArray = XMFloat2ToArray(data.uvs[texCoordIndex]);
    const std::array<float, 3> normalArray = XMFloat3ToArray(data.normals[normalIndex]);
    
    SimpleVertex vertex(positionArray, UVArray, normalArray);
    data.vertexData.push_back(vertex);
    data.indexData.push_back(static_cast<unsigned int>(data.vertexData.size() - 1));
    
}

void loadTexture(const std::string path, 
        std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, ID3D11Device* device) {

    if (loadedTextures.find(path) == loadedTextures.end())
    {
        ShaderResourceTextureD3D11 srv(device, path.c_str());
        loadedTextures[path] = srv;
        
    }
        
}

void ParseMTL(const std::string director, const std::string& dataSection, ParseData& data, 
            std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, ID3D11Device* device)
{
    std::string fileData;
    std::string path = director + dataSection;
    ReadFile(path, fileData);
    std::istringstream fileStream(fileData);


    std::string line;
    std::string prefix;
    ParsedMaterial* material = nullptr; // Initialize the material pointer

    while (std::getline(fileStream, line)) {

        std::istringstream lineStream(line);
        std::string keyword;
        lineStream >> keyword;

        if (keyword == "newmtl") {
            // first fall
            if (material != nullptr && material->mapKd == "")
            {

                material->mapKd = "../Textures/2dPng/RockTexture.png";
            }
            if (material != nullptr && material->mapKa == "")
            {

                material->mapKa = "../Textures/DefualtTexture/whiteKA.png";
            }
            if (material != nullptr && material->shininess == 0.0 )
            {
                material->shininess = 250.000000;
            }
            if (material != nullptr && material->mapKs == "")
            {
                material->mapKs = "../Textures/DefualtTexture/greenKs.png";
            }
            // load texture 
            if (material != nullptr)
            {
                loadTexture(material->mapKd, loadedTextures, device);
            }
            

            std::string material_name;
            lineStream >> material_name;
            material = new ParsedMaterial();
            if (data.parsedFaces.find(material_name) == data.parsedFaces.end())
            {
               auto& newMaterial = data.parsedMaterials[material_name];
               material = &newMaterial;
            }

            data.currentSubMeshMaterial = material_name;
        }
        else if (data.currentSubMeshMaterial != "") {
            std::string mapTexture;
            if (keyword == "map_Ka") {
                lineStream >> mapTexture;
                material->mapKa = director + mapTexture;
                
            }
            else if (keyword == "map_Kd") {
                lineStream >> mapTexture;
                material->mapKd = director + mapTexture;
            }
            else if (keyword == "map_Ks") {
                lineStream >> mapTexture;
                material->mapKs = director + mapTexture;
            }
            else if (keyword == "Ns") {
                lineStream >> material->shininess;
                if (material->shininess == 0.0)
                {
                    material->shininess = 250.000000;
                }
            }
        }

    }

    if (material != nullptr && material->mapKd == "")
    {

        material->mapKd = "../Textures/2dPng/RockTexture.png";
    }
    if (material != nullptr && material->mapKa == "")
    {

        material->mapKa = "../Textures/DefualtTexture/whiteKA.png";
    }
    if (material != nullptr && material->shininess == 0.0 )
    {
        material->shininess = 250.000000;
    }
    if (material != nullptr && material->mapKs == "")
    {
        material->mapKs = "../Textures/DefualtTexture/greenKs.png";
    }

    data.currentSubMeshMaterial = "";
    if (material != nullptr)
    {
        loadTexture(material->mapKd, loadedTextures, device);
    }
    
    /*loadTexture(material->mapKd, loadedTextures, device);
    loadTexture(material->mapKa, loadedTextures, device);
    loadTexture(material->mapKs, loadedTextures, device);*/
}



void PushbackCurrentSubmesh(ParseData& data, std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures)
{
    SubMeshInfo toAdd;
    toAdd.startIndexValue = data.currentSubMeshStartIndex;
    toAdd.nrOfIndicesInSubMesh = data.indexData.size() - toAdd.startIndexValue;
    
    toAdd.diffuseTextureSRV = loadedTextures[data.parsedMaterials[data.currentSubMeshMaterial].mapKd].GetSRV();
    data.finishedSubMeshes.push_back(toAdd);
}

void ParseLine(const std::string director, const std::string& line, ParseData& data, 
        std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, ID3D11Device* device)
{
    std::istringstream lineStream(line);
    std::string prefix;

    // Extract the prefix (e.g., "v", "vt", "vn", "f");
    lineStream >> prefix;
    // Get the rest of the line (remaining data after the prefix)
    std::string dataSection;
    std::getline(lineStream, dataSection);

    // Remove leading spaces from dataSection
    if (!dataSection.empty() && std::isspace(dataSection[0])) {
        dataSection.erase(0, dataSection.find_first_not_of(" \t"));
    }

    if (prefix == "v") {
        ParsePosition(dataSection, data);
    }
    else if (prefix == "vn") {
        ParseNormal(dataSection, data);
    }
    else if (prefix == "vt") {
        ParseTextureCoordinates(dataSection, data);
    }
    else if (prefix == "f") {

        ParseFaces(dataSection, data);
    }
    else if (prefix == "mtllib")
    {
        ParseMTL(director, dataSection, data, loadedTextures, device);
    }
    else if (prefix == "usemtl") {
        if (data.currentSubMeshMaterial != "")
        {
            PushbackCurrentSubmesh(data, loadedTextures);
        }
        data.currentSubMeshMaterial = dataSection;
        data.currentSubMeshStartIndex = data.indexData.size();


    }

}


void ParseOBJ(const std::string director, const std::string& identifier, const std::string& contents,
        std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures, 
        std::unordered_map<std::string, MeshD3D11>& loadedMeshes, ID3D11Device* device)
{
    std::istringstream lineStream(contents);
    ParseData data;

    std::string line;
    while (std::getline(lineStream, line)) {
        ParseLine(director, line, data, loadedTextures, device);
    }

    PushbackCurrentSubmesh(data, loadedTextures);

    MeshData send;
    send.vertexInfo.nrOfVerticesInBuffer = data.vertexData.size();
    send.vertexInfo.sizeOfVertex = sizeof(SimpleVertex);
    send.vertexInfo.vertexData = data.vertexData.data();

    send.indexInfo.nrOfIndicesInBuffer = data.indexData.size();
    send.indexInfo.indexData = data.indexData.data();

    send.subMeshInfo = data.finishedSubMeshes;
    send.vertexPositions = data.positions;

    
    MeshD3D11 toAdd;
    toAdd.Initialize(device, send);
    loadedMeshes[identifier] = std::move(toAdd);

}

const MeshD3D11* GetMesh(const std::string director, const std::string& identifier,
    std::unordered_map<std::string, ShaderResourceTextureD3D11>& loadedTextures,
    std::unordered_map<std::string, MeshD3D11>& loadedMeshes, ID3D11Device* device)
{   
    std::string fileData;
    if (loadedMeshes.find(identifier) == loadedMeshes.end())
    {
        ReadFile(director + identifier, fileData);
        ParseOBJ(director, identifier, fileData, loadedTextures, loadedMeshes, device);
    }
    
    return &loadedMeshes[identifier];
}

