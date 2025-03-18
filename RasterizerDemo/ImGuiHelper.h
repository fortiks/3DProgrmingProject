#pragma once

#include <Windows.h>
#include <d3d11.h>

struct VertexConstantBuffer
{
	float colourModifier[3] = { 0.0f, 0.0f, 0.0f };
	float padding = 0; // Remember packing rules in constant buffers!
	float positionModifer[3] = { 0.0f, 0.0f, 0.0f };
};


struct IMGuiVariables
{
	int isShadow;
	int padding[3]; // padding
};

enum CameraType { MAIN_CAMERA, SpotLight_CAMERA1, SpotLight_CAMERA2, cubeCamera0, cubeCamera1
, cubeCamera2 ,cubeCamera3, cubeCamera4, cubeCamera5
};

void SetupImGui(HWND windowHandle, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

void StartImGuiFrame();


void ImGuiModifying(VertexConstantBuffer& toModify, ID3D11DeviceContext* context, ID3D11Buffer* buffer, 
			CameraType& currentCamera, IMGuiVariables& IMGuivariables);

void EndImGuiFrame();