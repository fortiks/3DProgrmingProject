#include "ImGuiHelper.h"

#include "..\imgui\imgui.h"
#include "..\imgui\backends\imgui_impl_win32.h"
#include "..\imgui\backends\imgui_impl_dx11.h"

void SetupImGui(HWND windowHandle, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(windowHandle);
	ImGui_ImplDX11_Init(device, deviceContext);
}

void StartImGuiFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}



void ImGuiModifying(VertexConstantBuffer& toModify, ID3D11DeviceContext* context, ID3D11Buffer* buffer, 
		CameraType& currentCamera, IMGuiVariables& IMGuivariables)
{
	bool begun = ImGui::Begin("Modifiers");
	if (begun)
	{
		ImGui::ColorEdit3("Colour modifier", toModify.colourModifier);
		ImGui::SliderFloat("X pos modifier", &toModify.positionModifer[0], -0.5f, 0.5f);
		ImGui::InputFloat("Y pos modifier", &toModify.positionModifer[1], 0.1f);


		if (ImGui::Button("Main Camera")) currentCamera = MAIN_CAMERA;
		if (ImGui::Button("Spotlight Camera1")) currentCamera = SpotLight_CAMERA1;
		if (ImGui::Button("Spotlight Camera2")) currentCamera = SpotLight_CAMERA2;

		if (ImGui::Button("cubeCamera0")) currentCamera = cubeCamera0;
		if (ImGui::Button("cubeCamera1")) currentCamera = cubeCamera1;
		if (ImGui::Button("cubeCamera2")) currentCamera = cubeCamera2;
		if (ImGui::Button("cubeCamera3")) currentCamera = cubeCamera3;
		if (ImGui::Button("cubeCamera4")) currentCamera = cubeCamera4;
		if (ImGui::Button("cubeCamera5")) currentCamera = cubeCamera5;


		bool shadowEnabled = (IMGuivariables.isShadow != 0);
		if (ImGui::Checkbox("Enable Shadows", &shadowEnabled))
		{
			IMGuivariables.isShadow = shadowEnabled ? 1 : 0;
		}
		

	}
	ImGui::End();


	D3D11_MAPPED_SUBRESOURCE  mapped;
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &toModify, sizeof(VertexConstantBuffer));
	context->Unmap(buffer, 0);
}

void EndImGuiFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
