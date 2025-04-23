#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h> 
#include <fstream>
#include <iostream>

#include "WindowHelper.h"
#include "frustumRender.h"


// buffers
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "ConstantBufferD3D11.h"

// obj paser
#include "parsedData.h"

// deferredRendering
#include "DeferredRendering.h"

// Textures 
#include "SamplerD3D11.h"
#include "ShaderResourceTextureD3D11.h"

// grapichs background
#include "WindowRenderer.h"
#include "Graphics.h"
#include "InputLayoutD3D11.h"
#include "ShaderD3D11.h"

// scene 
#include "SceneD3D11.h"

// camera 
#include "InputManager.h"
#include "CameraD3D11.h"

// time
#include <chrono>
#include <thread>  

// Light 
#include "SpotLightCollectionD3D11.h"
#include "DirectionalLightCollectionD3D11.h"

// imgui
#include "..\imgui\imgui.h"
#include "..\imgui\backends\imgui_impl_win32.h"
#include "..\imgui\backends\imgui_impl_dx11.h"
#include "ImGuiHelper.h"


#include "CubicMappingD3D11.h"

#include "ParticleSystemD3D11.h"


DirectX::XMFLOAT3 sunMovement(DirectX::XMMATRIX& world, float dt)
{
	// Sun movement parameters
	static float angle = 0.0f;  // Start angle
	const float radius = 30.0f; // Radius of the half-circle
	const float speed = 0.25f;   // Rotation speed 

	// Update the sun's position
	float x = radius * cosf(angle); // X position (moves left to right)
	float y = radius * sinf(angle); // Y position (moves up and down)
	float z = 0.0f;
	

	DirectX::XMMATRIX baseTransform = world;
	world = DirectX::XMMatrixTranslation(x, y, z-35);

	angle += speed * dt; // deltaTime ensures smooth motion

// Reset angle when the sun completes the half-circle (π radians = 180°)
	if (angle >= DirectX::XM_PI) {
		angle = 0.0f; // Reset back to the start
	}

	DirectX::XMFLOAT3 sunPosition(x, y, z-35);
	DirectX::XMFLOAT3 direction(0.0f, 0.0f, 0.0f);

	
	direction.x = -sunPosition.x;
	direction.y = -sunPosition.y;
	direction.z = -sunPosition.z;

	// Normalize the direction vector
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
	dir = DirectX::XMVector3Normalize(dir);
	DirectX::XMStoreFloat3(&direction, dir);

	return direction;

}

void CircleMovement(DirectX::XMMATRIX& world, float dt, float radius = 5.0f, float speed = 1.0f)
{
	static float angle = 0.0f; // Track rotation over time
	angle += speed * dt;       


	// Calculate new position
	float x = radius * cos(angle);
	float z = radius * sin(angle);
	float y = 0.0f; // Keep Y constant for a horizontal circle

	// Update world matrix (translation)
	world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f)  * DirectX::XMMatrixTranslation(x + 5, y + 1.5, z - 4);
}


ID3D11Buffer* CreateVertexConstantBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = 32;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	VertexConstantBuffer dummy;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &dummy;
	data.SysMemPitch = data.SysMemSlicePitch = 0;

	ID3D11Buffer* toReturn;
	device->CreateBuffer(&desc, &data, &toReturn);

	return toReturn;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;
	HWND window;
	
	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		MessageBoxA(NULL, "Failed to setup window!", "Error", MB_ICONERROR);
		return -1;
	} 
	
	
	// === setup D3D11 ===
	Graphics graphics(window, WIDTH, HEIGHT);
	WindowRenderer renderer;
	renderer.Initialize(&graphics);
	
	// === shaders in use ===
	ShaderD3D11 pixelShader;
	pixelShader.Initialize(graphics.GetDevice(), 
		ShaderType::PIXEL_SHADER, L"..\\x64\\Debug\\PixelShader.cso");

	ShaderD3D11 vertexShader;
	vertexShader.Initialize(graphics.GetDevice(), 
		ShaderType::VERTEX_SHADER, L"..\\x64\\Debug\\vertexshader.cso");

	ShaderD3D11 vertexShaderLight;
	vertexShaderLight.Initialize(graphics.GetDevice(), 
		ShaderType::VERTEX_SHADER, L"..\\x64\\Debug\\VertexShaderDepthPass.cso");

	ShaderD3D11 computerShader;
	computerShader.Initialize(graphics.GetDevice(), ShaderType::COMPUTE_SHADER, L"..\\x64\\Debug\\ComputerShader.cso");

	ShaderD3D11 computerShaderCubeMapping;
	computerShaderCubeMapping.Initialize(graphics.GetDevice(), ShaderType::COMPUTE_SHADER, 
		L"..\\x64\\Debug\\ComputeShaderCubemaping.cso");

	ShaderD3D11 TessellationHullShader;
	TessellationHullShader.Initialize(graphics.GetDevice(), 
		ShaderType::HULL_SHADER, L"..\\x64\\Debug\\TessellationHullShader.cso");

	ShaderD3D11 TessellationDomainShader;
	TessellationDomainShader.Initialize(graphics.GetDevice(), 
		ShaderType::DOMAIN_SHADER, L"..\\x64\\Debug\\TessellationDomainShader.cso");

	

	InputLayoutD3D11 inputLayout1; // put this in the vertexShader class 
	inputLayout1.AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayout1.AddInputElement("UV", DXGI_FORMAT_R32G32_FLOAT);
	inputLayout1.AddInputElement("Normal", DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayout1.AddInputElement("COLOUR", DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayout1.FinalizeInputLayout(graphics.GetDevice(), vertexShader.GetShaderByteData(), vertexShader.GetShaderByteSize());

	// frustum 
	ShaderD3D11 FrustumVertexShader;
	FrustumVertexShader.Initialize(graphics.GetDevice(),
		ShaderType::VERTEX_SHADER, L"..\\x64\\Debug\\frustumVertexShader.cso");

	ShaderD3D11 FrustumpixelShader;
	FrustumpixelShader.Initialize(graphics.GetDevice(),
		ShaderType::PIXEL_SHADER, L"..\\x64\\Debug\\frustumPixelShader.cso");

	InputLayoutD3D11 inputLayoutFrustum; // put this in the vertexShader class 
	inputLayoutFrustum.AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayoutFrustum.AddInputElement("COLOUR", DXGI_FORMAT_R32G32B32_FLOAT);
	inputLayoutFrustum.FinalizeInputLayout(graphics.GetDevice(), FrustumVertexShader.GetShaderByteData(), FrustumVertexShader.GetShaderByteSize());

	

	SetupImGui(window, graphics.GetDevice(), graphics.GetContext());

	// === Camera ===
	ProjectionInfo projectionInfo;
	projectionInfo.fovAngleY = DirectX::XMConvertToRadians(90.0f);
	projectionInfo.aspectRatio = static_cast<float>(WIDTH) / HEIGHT;
	projectionInfo.nearZ = 0.1f;
	projectionInfo.farZ = 100.0f;
	InputManager inputManager;
	CameraD3D11 camera(graphics.GetDevice(), projectionInfo, { 5.0f, 3.0f, 5.0f });
	camera.RotateRight(DirectX::XM_PI);
	camera.RotateForward(DirectX::XM_PI);
	
	float mouseSensitivity = 0.001f; // Adjust for desired speed

	CameraType currentCamera = MAIN_CAMERA;
	
	VertexConstantBuffer vertexConstantBufferData;
	ID3D11Buffer* vertexConstantBufferInterface = CreateVertexConstantBuffer(graphics.GetDevice());

	// === Scene Models ===;
	ParseData parse;
	std::unordered_map<std::string, ShaderResourceTextureD3D11> loadedTextures;
	std::unordered_map<std::string, MeshD3D11> loadedMeshes; 
	
	const MeshD3D11* meshRubiksCube = GetMesh("../Textures/RubiksCube/", "RubiksCube.obj", 
		loadedTextures, loadedMeshes, graphics.GetDevice());
	
	const MeshD3D11* meshCube = GetMesh("../Textures/Cube/", "Cube.obj", 
		loadedTextures, loadedMeshes, graphics.GetDevice());

	const MeshD3D11* meshGround = GetMesh("../Textures/Road/", "ground.obj", 
		loadedTextures, loadedMeshes, graphics.GetDevice());

	const MeshD3D11* meshSun = GetMesh("../Textures/Sun/", "Sun.obj",
		loadedTextures, loadedMeshes, graphics.GetDevice());

	const MeshD3D11* meshLamp = GetMesh("../Textures/street lamp/", "StreetLamp.obj",
		loadedTextures, loadedMeshes, graphics.GetDevice());

	const MeshD3D11* meshBarrelStove = GetMesh("../Textures/barrel_stove/", "barrel_stove_4k.obj",  
		loadedTextures, loadedMeshes, graphics.GetDevice());

	const MeshD3D11* meshBrickSphere = GetMesh("../Textures/Sun/", "BrickSphere.obj",  
		loadedTextures, loadedMeshes, graphics.GetDevice());

	// === Scenes ===
	SceneD3D11 firstScene;


	// === Scene Objects ===
	std::shared_ptr<SamplerD3D11> sampler = std::make_shared<SamplerD3D11>();
	sampler->Initialize(graphics.GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP);

	// ground 
	std::unique_ptr<SceneObjectD3D11> objectGround = std::make_unique<SceneObjectD3D11>();
	objectGround->Initialize(meshGround, sampler, graphics.GetDevice());
	objectGround->world = DirectX::XMMatrixScaling(2, 1, 2) * DirectX::XMMatrixTranslation(-3, 0, 5);

	// Object 1
	std::unique_ptr<SceneObjectD3D11> objectRubiksCube = std::make_unique<SceneObjectD3D11>();
	objectRubiksCube->Initialize(meshRubiksCube, sampler, graphics.GetDevice());
	objectRubiksCube->world = DirectX::XMMatrixScaling(0.4f, 0.4f, 0.4f) * DirectX::XMMatrixTranslation(9, 1.5, -1);;

	// Object 2 
	std::unique_ptr<SceneObjectD3D11> objectCube = std::make_unique<SceneObjectD3D11>();
	objectCube->Initialize(meshCube, sampler, graphics.GetDevice());
	objectCube->world = DirectX::XMMatrixTranslation(13, 1, -8);

	// Object 3 
	std::unique_ptr<SceneObjectD3D11> objectRubiksCube2 = std::make_unique<SceneObjectD3D11>();
	objectRubiksCube2->Initialize(meshRubiksCube, sampler, graphics.GetDevice());
	objectRubiksCube2->world = DirectX::XMMatrixScaling(0.4f, 0.4f, 0.4f) * 
		DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180)) * DirectX::XMMatrixTranslation(4, 12, -4);

	/*
	DirectX::XMMatrixScaling(0.4f, 0.4f, 0.4f) * 
		DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180)) * DirectX::XMMatrixTranslation(10, 1.5, -8);*/

	
	// Object 4 
	std::unique_ptr<SceneObjectD3D11> objectSphere = std::make_unique<SceneObjectD3D11>();
	objectSphere->Initialize(meshSun, sampler, graphics.GetDevice());
	objectSphere->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(15.0f, 2.0f, -4.0f);

	// Object 5
	std::unique_ptr<SceneObjectD3D11> objectStreetLamp = std::make_unique<SceneObjectD3D11>();
	objectStreetLamp->Initialize(meshLamp, sampler, graphics.GetDevice());
	objectStreetLamp->world = DirectX::XMMatrixScaling(2, 2, 2) *
		DirectX::XMMatrixTranslation(1.0f, 0.0f, -8.0f);


	// object 6 
	std::unique_ptr<SceneObjectD3D11> objectBarrelStove = std::make_unique<SceneObjectD3D11>();
	objectBarrelStove->Initialize(meshBarrelStove, sampler, graphics.GetDevice());
	objectBarrelStove->world = DirectX::XMMatrixScaling(3, 3, 3) *
		DirectX::XMMatrixTranslation(15.0f, 0.0f, -1.0f);

	// 7
	std::unique_ptr<SceneObjectD3D11> objectSphere3 = std::make_unique<SceneObjectD3D11>();
	objectSphere3->Initialize(meshBrickSphere, sampler, graphics.GetDevice(), RenderMode::Texture);
	objectSphere3->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(5.0f, 2.0f, 0.0f);

	// 8
	std::unique_ptr<SceneObjectD3D11> objectSphere4 = std::make_unique<SceneObjectD3D11>();
	objectSphere4->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere4->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(-10.0f, 10.0f, 0.0f);

	//9
	std::unique_ptr<SceneObjectD3D11> objectSphere5 = std::make_unique<SceneObjectD3D11>();
	objectSphere5->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere5->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(0.0f, 10.0f, -3.0f);

	//10
	std::unique_ptr<SceneObjectD3D11> objectSphere6 = std::make_unique<SceneObjectD3D11>();
	objectSphere6->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere6->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(10.0f, 12.0f, -5.0f);

	//11
	std::unique_ptr<SceneObjectD3D11> objectSphere7 = std::make_unique<SceneObjectD3D11>();
	objectSphere7->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere7->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(5.0f, 10.0f, 0.0f);

	//12
	std::unique_ptr<SceneObjectD3D11> objectSphere8 = std::make_unique<SceneObjectD3D11>();
	objectSphere8->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere8->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(12, 10.0f, -10.0f);

	//13
	std::unique_ptr<SceneObjectD3D11> objectSphere9 = std::make_unique<SceneObjectD3D11>();
	objectSphere9->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere9->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(4.0f, 15.0f, -7.0f);

	//14
	std::unique_ptr<SceneObjectD3D11> objectSphere10 = std::make_unique<SceneObjectD3D11>();
	objectSphere10->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere10->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(-2.0f, 12.0f, -1.0f);

	//15
	std::unique_ptr<SceneObjectD3D11> objectSphere11 = std::make_unique<SceneObjectD3D11>();
	objectSphere11->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere11->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(8.0f, 10.0f, -12.0f);


	//16
	std::unique_ptr<SceneObjectD3D11> objectSphere12 = std::make_unique<SceneObjectD3D11>();
	objectSphere12->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere12->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(8.0f, 3.0f, -15.0f);

	//17
	std::unique_ptr<SceneObjectD3D11> objectSphere13 = std::make_unique<SceneObjectD3D11>();
	objectSphere13->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere13->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(6.0f, 3.0f, -15.0f);

	//18
	std::unique_ptr<SceneObjectD3D11> objectSphere14 = std::make_unique<SceneObjectD3D11>();
	objectSphere14->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere14->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(0.0f, 3.0f, -15.0f);

	//19
	std::unique_ptr<SceneObjectD3D11> objectSphere15 = std::make_unique<SceneObjectD3D11>();
	objectSphere15->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere15->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(0.0f, 12.0f, -15.0f);

	//20
	std::unique_ptr<SceneObjectD3D11> objectSphere16 = std::make_unique<SceneObjectD3D11>();
	objectSphere16->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere16->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(20.0f, 10.0f, -12.0f);

	//21
	std::unique_ptr<SceneObjectD3D11> objectSphere17 = std::make_unique<SceneObjectD3D11>();
	objectSphere17->Initialize(meshSun, sampler, graphics.GetDevice(), RenderMode::Color);
	objectSphere17->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(-35.0f, -10.0f, -12.0f);


	// === Moving objects ===
	// Object 0 SUN
	std::unique_ptr<SceneObjectD3D11> objectSun = std::make_unique<SceneObjectD3D11>();
	objectSun->Initialize(meshSun, sampler, graphics.GetDevice());
	objectSun->world = DirectX::XMMatrixScaling(1, 1, 1) *
		DirectX::XMMatrixTranslation(0.0f, 1.5f, -15.0f);


	// Object 1
	std::unique_ptr<SceneObjectD3D11> objectSphere2 = std::make_unique<SceneObjectD3D11>();
	objectSphere2->Initialize(meshSun, sampler, graphics.GetDevice());
	objectSphere2->world = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(15.0f, 2.0f, -4.0f);


	//  === ADD objects to Scences ===
	firstScene.AddObject(std::move(objectGround));
	firstScene.AddObject(std::move(objectRubiksCube));
	firstScene.AddObject(std::move(objectCube));
	firstScene.AddObject(std::move(objectRubiksCube2));
	firstScene.AddObject(std::move(objectSphere));
	firstScene.AddObject(std::move(objectStreetLamp));
	firstScene.AddObject(std::move(objectBarrelStove));
	firstScene.AddObject(std::move(objectSphere3));
	firstScene.AddObject(std::move(objectSphere4));
	firstScene.AddObject(std::move(objectSphere5));
	firstScene.AddObject(std::move(objectSphere6));
	firstScene.AddObject(std::move(objectSphere7));
	firstScene.AddObject(std::move(objectSphere8));
	firstScene.AddObject(std::move(objectSphere9));
	firstScene.AddObject(std::move(objectSphere10));
	firstScene.AddObject(std::move(objectSphere11));
	firstScene.AddObject(std::move(objectSphere12));
	firstScene.AddObject(std::move(objectSphere13));
	firstScene.AddObject(std::move(objectSphere14));
	firstScene.AddObject(std::move(objectSphere15));
	firstScene.AddObject(std::move(objectSphere16));
	firstScene.AddObject(std::move(objectSphere17));
	


	firstScene.AddMovingObject(std::move(objectSun));
	firstScene.AddMovingObject(std::move(objectSphere2));
	

	firstScene.getObject(0)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(1)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(2)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(3)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(4)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(5)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(6)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(7)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(8)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(9)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(10)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(11)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(12)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(13)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(14)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(15)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(16)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(17)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(18)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(19)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(20)->updateConstBuffer(graphics.GetContext());
	firstScene.getObject(21)->updateConstBuffer(graphics.GetContext());
	

	firstScene.getObject(0)->updateBoundingBox(firstScene.getObject(0)->world);
	firstScene.getObject(1)->updateBoundingBox(firstScene.getObject(1)->world);
	firstScene.getObject(2)->updateBoundingBox(firstScene.getObject(2)->world);
	firstScene.getObject(3)->updateBoundingBox(firstScene.getObject(3)->world);
	firstScene.getObject(4)->updateBoundingBox(firstScene.getObject(4)->world);
	firstScene.getObject(5)->updateBoundingBox(firstScene.getObject(5)->world);
	firstScene.getObject(6)->updateBoundingBox(firstScene.getObject(6)->world);
	firstScene.getObject(7)->updateBoundingBox(firstScene.getObject(7)->world);
	firstScene.getObject(8)->updateBoundingBox(firstScene.getObject(8)->world);
	firstScene.getObject(9)->updateBoundingBox(firstScene.getObject(9)->world);
	firstScene.getObject(10)->updateBoundingBox(firstScene.getObject(10)->world);
	firstScene.getObject(11)->updateBoundingBox(firstScene.getObject(11)->world);
	firstScene.getObject(12)->updateBoundingBox(firstScene.getObject(12)->world);
	firstScene.getObject(13)->updateBoundingBox(firstScene.getObject(13)->world);
	firstScene.getObject(14)->updateBoundingBox(firstScene.getObject(14)->world);
	firstScene.getObject(15)->updateBoundingBox(firstScene.getObject(15)->world);
	firstScene.getObject(16)->updateBoundingBox(firstScene.getObject(16)->world);
	firstScene.getObject(17)->updateBoundingBox(firstScene.getObject(17)->world);
	firstScene.getObject(18)->updateBoundingBox(firstScene.getObject(18)->world);
	firstScene.getObject(19)->updateBoundingBox(firstScene.getObject(19)->world);
	firstScene.getObject(20)->updateBoundingBox(firstScene.getObject(20)->world);
	firstScene.getObject(21)->updateBoundingBox(firstScene.getObject(21)->world);
	
	//  === Culling of Objects ===
	DirectX::BoundingFrustum cameraFrustum;  
	DirectX::BoundingFrustum::CreateFromMatrix(cameraFrustum, DirectX::XMMatrixTranspose(camera.GetProjectionMatrix())); // 

	DirectX::XMMATRIX  cameraMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(camera.GetViewMatrix())); // world space

	DirectX::BoundingFrustum cameraWorldFrustum;
	cameraFrustum.Transform(cameraWorldFrustum, cameraMatrix);

	DirectX::XMFLOAT3 corners[8];
	cameraWorldFrustum.GetCorners(corners);
	FrustumRenderer frustumRenderer(graphics.GetDevice(), corners);


	firstScene.InitializeOcTree(20, 20);
	bool WriteOcTreeInfo = false;
	if (WriteOcTreeInfo)
	{
		std::string WriteOcTree = firstScene.DrawOcTree();
		std::ofstream outFile("OcTree visulation.txt");

		if (outFile.is_open())
		{
			// Write the DOT data to the file
			outFile << WriteOcTree;
			outFile.close();
			std::string typeString = "Graphviz DOT file successfully written to OcTree visulation.txt";
			MessageBoxA(NULL, typeString.c_str(), "Error", MB_ICONERROR);

		}
		else
		{
			std::cerr << "Failed to open file " << "OcTree visulation.txt" << " for writing." << std::endl;
			throw std::runtime_error("Could not create texture cube");

			std::string typeString = "Failed to open file OcTree visulation.txt";
			MessageBoxA(NULL, typeString.c_str(), "Error", MB_ICONERROR);
		}

	}
	


	// === SpotLights ===

	SpotLightData spotLightData;
	spotLightData.shadowMapInfo.textureDimension = 1024;
	// 1
	PerLightInfo perLightData;
	perLightData.angle = DirectX::XMConvertToRadians(35.0f);
	perLightData.initialPosition = DirectX::XMFLOAT3(2.5f, 7.0f, -8.0f); 
	perLightData.projectionFarZ = 100.0f;
	perLightData.projectionNearZ = 1.0f;
	perLightData.rotationX = 0; // DirectX::XMConvertToRadians(180.0f);
	perLightData.rotationY = DirectX::XMConvertToRadians(90.0f);
	perLightData.colour = {1,1,0};

	spotLightData.perLightInfo.push_back(perLightData);
	
	// 2
	PerLightInfo perLightData2;
	perLightData2.angle = DirectX::XMConvertToRadians(30.0f);
	perLightData2.initialPosition = DirectX::XMFLOAT3(15.0f, 5.0f, -1.0f);
	perLightData2.projectionFarZ = 100.0f;
	perLightData2.projectionNearZ = 0.1f;
	perLightData2.rotationX = 0; // DirectX::XMConvertToRadians(180.0f);
	perLightData2.rotationY = DirectX::XMConvertToRadians(180.0f);
	perLightData2.colour = { 0,0,1 };

	spotLightData.perLightInfo.push_back(perLightData2);

	// 3
	PerLightInfo perLightData3;
	perLightData3.angle = DirectX::XMConvertToRadians(35.0f);
	perLightData3.initialPosition = DirectX::XMFLOAT3(2.5f, 7.0f, -4.0f);
	perLightData3.projectionFarZ = 100.0f;
	perLightData3.projectionNearZ = 1.0f;
	perLightData3.rotationX = 0; // DirectX::XMConvertToRadians(180.0f);
	perLightData3.rotationY = DirectX::XMConvertToRadians(90.0f);
	perLightData3.colour = { 1,0,0 };

	spotLightData.perLightInfo.push_back(perLightData3);

	SpotLightCollectionD3D11 spotLights;
	spotLights.Initialize(graphics.GetDevice(), spotLightData);
	spotLights.UpdateLightBuffers(graphics.GetContext());


	// === Directional Light ===
	DirectionalLightData directionalLightData;
	directionalLightData.shadowMapInfo.textureDimension = 1024;

	PerDirectionalLightInfo perDirectionalLightData;
	perDirectionalLightData.angle = DirectX::XMConvertToRadians(35.0f);
	perDirectionalLightData.initialPosition = DirectX::XMFLOAT3(2.5f, 1.0f, 0.0f);
	perDirectionalLightData.projectionFarZ = 100.0f;
	perDirectionalLightData.projectionNearZ = 1.0f;
	perDirectionalLightData.rotationX = 0; // DirectX::XMConvertToRadians(180.0f);
	perDirectionalLightData.rotationY = DirectX::XMConvertToRadians(180.0f);
	perDirectionalLightData.colour = {1.0f, 1.0f, 0.9f};

	directionalLightData.perLightInfo.push_back(perDirectionalLightData);

	DirectionalLightCollectionD3D11 directionalLights;
	directionalLights.Initialize(graphics.GetDevice(), directionalLightData);

	// === DeferredRendering ===
	DeferredRenderer deferredRender(graphics.GetDevice(), WIDTH, HEIGHT);
	deferredRender.InitializeBackBuffer(graphics.GetDevice(), renderer.GetSwapChain());

	MSG msg = { };
	std::chrono::duration<float> dt; // time
	auto startTime = std::chrono::high_resolution_clock::now();
	auto endTime = std::chrono::high_resolution_clock::now();

	// === Tessellation ===
	TessellationBufferData tessellationBufferData;
	tessellationBufferData.TessMax = 32;
	tessellationBufferData.TessMin = 2;
	tessellationBufferData.DecayFactor = 0.2f;
	tessellationBufferData.CameraPosition = { 0,0,0};

	ConstantBufferD3D11 tessellationBuffer(graphics.GetDevice(), sizeof(TessellationBufferData), &tessellationBufferData);
	
	// displacement
	DisplacementStrength displacementStrength;
	displacementStrength.strength = 0.05;
	ConstantBufferD3D11 displacementBuffer(graphics.GetDevice(), sizeof(displacementStrength), &displacementStrength);


	ShaderResourceTextureD3D11 DisplacementMap;
	DisplacementMap.Initialize(graphics.GetDevice(), "../Textures/T_Brick_RGBA.PNG");  //brick_wall_Displacement_RGBA.png 

	SamplerD3D11 DisplacementMapSampler(graphics.GetDevice(), D3D11_TEXTURE_ADDRESS_BORDER);


	// === Cubic Mapping ===
	DirectX::XMFLOAT3 objectPos = { 2.0f, 4.0f, -4.0f };
	CubicMappingD3D11 cubicMapping(graphics.GetDevice(), graphics.GetContext(), 1024, 1024, objectPos);
	DeferredRenderer cubicDeferredRender(graphics.GetDevice(), 1024, 1024);
	ConstantBufferD3D11 cameraPosBuffer;
	cameraPosBufferData cameraPosBufferdata;
	
	cameraPosBufferdata.cameraPosition = camera.GetPosition();
	cameraPosBuffer.Initialize(graphics.GetDevice(), sizeof(cameraPosBufferData), &cameraPosBufferdata);

	// object for cubic mappingsadwfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
	SceneObjectD3D11 objectSphereCubic;
	objectSphereCubic.Initialize(meshSun, sampler, graphics.GetDevice());
	objectSphereCubic.world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f) *
		DirectX::XMMatrixTranslation(objectPos.x, objectPos.y, objectPos.z);


	// === Fire System ===
	ParticleSystemD3D11 fireSystem;
	FireParticle fireParticles[MAX_PARTICLES];
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		fireParticles[i].position = DirectX::XMFLOAT3(15.0f, 0.0f, -1.0f);
		float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI; // Random angle
		float speed = 0.7f + ((float)rand() / RAND_MAX) * 0.5f; // Random speed

		fireParticles[i].velocity = DirectX::XMFLOAT3(
			cos(angle) * 0.2f,  // Slight outward motion
			speed,              // Mostly upwards
			sin(angle) * 0.2f   // Slight outward motion
		);

		fireParticles[i].lifetime = 1.5f + ((float)rand() / RAND_MAX) * 1.0f; // 0.5s - 1.5s

		fireParticles[i].size = 0.1f + ((float)rand() / RAND_MAX) * 0.2f;

		fireParticles[i].color = DirectX::XMFLOAT4(1.0f, 0.3f, 0.0f, 1.0f);
	}
	 
	fireSystem.Initialize(graphics.GetDevice(), sizeof(FireParticle), MAX_PARTICLES, &fireParticles, false, true);
	FireParticleCameraData fireParticleCameraData;
	fireParticleCameraData.world = camera.GetViewProjectionMatrix();
	fireParticleCameraData.position = camera.GetPosition();

	ConstantBufferD3D11 fireParticlesCameraBuffer(graphics.GetDevice(), sizeof(FireParticleCameraData),
		&fireParticleCameraData);


	// debug 
	/*
	ID3D11Device* device = graphics.GetDevice();
	ID3D11Debug* debugInterface = nullptr;
	HRESULT hr = device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugInterface));

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to get D3D11 debug interface!");
	}
	*/

	// === IMGUI BUFFERS ===
	StructuredBufferD3D11 IMGUIControlBuffer;
	IMGuiVariables IMGuivariables;
	IMGuivariables.isShadow = 1; // true 
	IMGUIControlBuffer.Initialize(graphics.GetDevice(), sizeof(IMGuivariables), 1, &IMGuivariables, true);
	int hej = 6000;

	// FPS 
	float frameCount = 0;
	float elapsedTime = 0;
	float fps = 0;
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		endTime = std::chrono::high_resolution_clock::now();
		dt = endTime - startTime;
		startTime = endTime;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		StartImGuiFrame();
		// Inputs 
		inputManager.Update();
		
		if (inputManager.IsKeyPressed('F'))
		{
			ImGuiModifying(vertexConstantBufferData, graphics.GetContext(), vertexConstantBufferInterface, 
					currentCamera, IMGuivariables, displacementStrength.strength);
			IMGUIControlBuffer.UpdateBuffer(graphics.GetContext(), &IMGuivariables);
			inputManager.ShowCursorInClient();
		}
		else {
			inputManager.HideCursor();
			camera.HandleMouseMovement(inputManager.GetMouseDeltaX(), inputManager.GetMouseDeltaY(), mouseSensitivity);
			inputManager.CenterCursor(window);
			
		}
		if (inputManager.IsKeyPressed('W')) camera.MoveForward(0.1f);
		if (inputManager.IsKeyPressed('S')) camera.MoveForward(-0.1f);
		if (inputManager.IsKeyPressed('A')) camera.MoveRight(0.1f);
		if (inputManager.IsKeyPressed('D')) camera.MoveRight(-0.1f);

		if (inputManager.IsKeyPressed(VK_UP)) camera.RotateForward(0.1f);
		if (inputManager.IsKeyPressed(VK_DOWN)) camera.RotateUp(0.1f);
		if (inputManager.IsKeyPressed(VK_LEFT)) camera.RotateRight(0.1f);
		if (inputManager.IsKeyPressed(VK_RIGHT)) camera.RotateRight(-0.1f);

		// Update the camera constant buffer (done every frame)
		camera.UpdateInternalConstantBuffer(graphics.GetContext());

		// === Sun Movment  ===
		
		DirectX::XMFLOAT3 cameraDir = sunMovement(firstScene.getMovingObjects(0)->world, dt.count());

		CircleMovement(firstScene.getMovingObjects(1)->world, dt.count(), 10.0f, 0.5f);

		// update pos for objects
		/*
		firstScene.getObject(0)->updateBoundingBox(firstScene.getObject(0)->world);
		firstScene.getObject(1)->updateBoundingBox(firstScene.getObject(1)->world);
		firstScene.getObject(2)->updateBoundingBox(firstScene.getObject(2)->world);
		firstScene.getObject(3)->updateBoundingBox(firstScene.getObject(2)->world);
		firstScene.getObject(4)->updateBoundingBox(firstScene.getObject(3)->world);
		firstScene.getObject(5)->updateBoundingBox(firstScene.getObject(4)->world);
		firstScene.getObject(6)->updateBoundingBox(firstScene.getObject(6)->world);*/
		
		/*
		firstScene.getObject(0)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(1)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(2)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(3)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(5)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(6)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(7)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(8)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(9)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(10)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(11)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(12)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(13)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(14)->updateConstBuffer(graphics.GetContext());
		firstScene.getObject(15)->updateConstBuffer(graphics.GetContext());*/

		// Sun
		firstScene.getMovingObjects(0)->updateConstBuffer(graphics.GetContext());
		// movement 
		firstScene.getMovingObjects(1)->updateConstBuffer(graphics.GetContext());

		objectSphereCubic.updateConstBuffer(graphics.GetContext());
		
		//  Update Light 
		float lightDistance = 50.0f;
		camera.GetPosition();
		directionalLights.UpdateLightBuffers(graphics.GetContext(), cameraDir ); //camera.GetPosition()

		// === Tessellation Update ===
		tessellationBufferData.CameraPosition = camera.GetPosition();
		tessellationBuffer.UpdateBuffer(graphics.GetContext(), &tessellationBufferData,
			sizeof(TessellationBufferData));


		cameraMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(camera.GetViewMatrix()));
		cameraFrustum.Transform(cameraWorldFrustum, cameraMatrix);
		cameraWorldFrustum.GetCorners(corners);

		if (hej > 700)
		{
			frustumRenderer.UpdateVertexBuffer(graphics.GetContext(), corners); // unneccary but make it clear how the frustum work good for testing
			hej = 0;
		}
		hej++;

		// === Rendering ===
		
		// === depth shadow map pass  ===
		vertexShaderLight.BindShader(graphics.GetContext());
		D3D11_VIEWPORT shadowViewport = spotLights.GetViewport();
		graphics.GetContext()->RSSetViewports(1, &shadowViewport);
		graphics.GetContext()->IASetInputLayout(inputLayout1.GetInputLayout());
		graphics.GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics.GetContext()->PSSetShader(nullptr, nullptr, 0); // Do the same for other shaders if relevant
		ID3D11DepthStencilView* dsView;
		for (UINT lightIndex = 0; lightIndex < spotLights.GetNrOfLights(); ++lightIndex)
		{
			// camera
			ID3D11Buffer* cameraConstantBuffer = spotLights.GetLightCameraConstantBuffer(lightIndex);
			graphics.GetContext()->VSSetConstantBuffers(0, 1, &cameraConstantBuffer);

			dsView = spotLights.GetShadowMapDSV(lightIndex);
			graphics.GetContext()->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);
			graphics.GetContext()->OMSetRenderTargets(0, nullptr, dsView);

			firstScene.Render(graphics.GetContext(), vertexConstantBufferInterface, cameraWorldFrustum, true);
			
		}

		// Directional Lights
		shadowViewport = directionalLights.GetViewport();
		graphics.GetContext()->RSSetViewports(1, &shadowViewport);

		for (UINT lightIndex = 0; lightIndex < directionalLights.GetNrOfLights(); ++lightIndex)
		{
			// camera
			ID3D11Buffer* cameraConstantBuffer = directionalLights.GetLightCameraConstantBuffer(lightIndex);
			graphics.GetContext()->VSSetConstantBuffers(0, 1, &cameraConstantBuffer);

			dsView = directionalLights.GetShadowMapDSV(lightIndex);
			graphics.GetContext()->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);
			graphics.GetContext()->OMSetRenderTargets(0, nullptr, dsView);

			firstScene.Render(graphics.GetContext(), vertexConstantBufferInterface, cameraWorldFrustum, true);
		}


		dsView = nullptr;
		graphics.GetContext()->OMSetRenderTargets(0, nullptr, dsView);


		vertexShader.BindShader(graphics.GetContext());
		pixelShader.BindShader(graphics.GetContext());
		computerShader.BindShader(graphics.GetContext());
		TessellationHullShader.BindShader(graphics.GetContext());
		TessellationDomainShader.BindShader(graphics.GetContext());


		ID3D11Buffer* cameraMainConstantBuffer = nullptr;
		switch (currentCamera) {
		case MAIN_CAMERA:
			cameraMainConstantBuffer = camera.GetConstantBuffer();
			break;
		case SpotLight_CAMERA1:
			cameraMainConstantBuffer = spotLights.GetLightCameraConstantBuffer(0);
			break;
		case SpotLight_CAMERA2:
			cameraMainConstantBuffer = spotLights.GetLightCameraConstantBuffer(1);
			break;
		case cubeCamera0:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(0);
			break;
		case cubeCamera1:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(1);
			break;
		case cubeCamera2:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(2); 
			break;
		case cubeCamera3:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(3); 
			break;
		case cubeCamera4:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(4); 
			break;
		case cubeCamera5:
			cameraMainConstantBuffer = cubicMapping.GetCameraConstantBuffer(5); 
			break;
		}

		cameraPosBufferdata.cameraPosition = camera.GetPosition();
		cameraPosBuffer.UpdateBuffer(graphics.GetContext(), &cameraPosBufferdata, sizeof(cameraPosBufferData));

		renderer.ClearRenderTargets();
		deferredRender.BeginGeometryPass(graphics.GetContext(), renderer.GetDSV(0));
		renderer.Render(inputLayout1.GetInputLayout(), cameraMainConstantBuffer);

		// Tessellation Rendring
		ID3D11Buffer* tessellationConstantBuffer = tessellationBuffer.GetBuffer();
		ID3D11ShaderResourceView* DisplacementMapSRV = DisplacementMap.GetSRV();
		ID3D11SamplerState* DisplacementMapSamplerState = DisplacementMapSampler.GetSamplerState();
		displacementBuffer.UpdateBuffer(graphics.GetContext(), &displacementStrength, sizeof(displacementStrength));
		ID3D11Buffer* displacementbuffer = displacementBuffer.GetBuffer();
			
		graphics.GetContext()->HSSetConstantBuffers(0, 1, &tessellationConstantBuffer);
		graphics.GetContext()->DSSetShaderResources(0, 1, &DisplacementMapSRV);
		graphics.GetContext()->DSSetSamplers(0, 1, &DisplacementMapSamplerState);
		graphics.GetContext()->DSSetConstantBuffers(1, 1, &displacementbuffer);
		
		firstScene.Render(graphics.GetContext(), vertexConstantBufferInterface, cameraWorldFrustum);
		
		deferredRender.EndGeometryPass(graphics.GetContext());
		
		// === computer pass ===
		spotLights.BindSpotlights(graphics.GetContext());
		directionalLights.BindDirectionallights(graphics.GetContext());

		ID3D11ShaderResourceView* IMGuiSRV = IMGUIControlBuffer.GetSRV(); // computer IMGUI
		graphics.GetContext()->CSSetShaderResources(5, 1, &IMGuiSRV); // Bind SRV to slot 5
	
		deferredRender.RenderLightingPass(graphics.GetContext(), computerShader.GetComputeShader());

		spotLights.UnBindSportLights(graphics.GetContext());
		directionalLights.UnBindDirectionalLights(graphics.GetContext());


		// Rendering of cube map
		
		cubicMapping.RenderReflectiveObject(graphics.GetContext(), cubicDeferredRender, spotLights, directionalLights,
			firstScene, tessellationConstantBuffer, DisplacementMapSRV, DisplacementMapSamplerState,
			vertexConstantBufferInterface, IMGuiSRV, computerShaderCubeMapping.GetComputeShader(),
			inputLayout1.GetInputLayout(), cameraWorldFrustum);

		renderer.SetRenderTarget();
		graphics.Render();
		cubicMapping.bindCubeMapping(graphics.GetContext(), objectSphereCubic, cameraMainConstantBuffer,
			cameraPosBuffer.GetBuffer(), vertexConstantBufferInterface);

		fireParticleCameraData.world = camera.GetViewProjectionMatrix();
		fireParticleCameraData.position = camera.GetPosition();
		fireParticlesCameraBuffer.UpdateBuffer(graphics.GetContext(), &fireParticleCameraData, sizeof(FireParticleCameraData));
		
		renderer.SetRenderTarget();
		fireSystem.bindSystem(graphics.GetContext());
		fireSystem.drawSystem(graphics.GetContext(), fireParticlesCameraBuffer.GetBuffer());

		/*
		FrustumVertexShader.BindShader(graphics.GetContext());
		FrustumpixelShader.BindShader(graphics.GetContext());
		
		frustumRenderer.Render(graphics.GetContext(), inputLayoutFrustum.GetInputLayout(), camera.GetConstantBuffer());*/

		vertexShader.BindShader(graphics.GetContext());
		pixelShader.BindShader(graphics.GetContext());
		computerShader.BindShader(graphics.GetContext());
		TessellationHullShader.BindShader(graphics.GetContext());
		TessellationDomainShader.BindShader(graphics.GetContext());

		
		
		renderer.SetRenderTarget();
		frameCount++;
		elapsedTime += dt.count();
		if (elapsedTime >= 1.0f) {  // Update FPS every second
			fps = frameCount;
			frameCount = 0;
			elapsedTime = 0.0f;
		}
		ImGui::Text("FPS: %.1f", fps);
		//renderer.RenderWithWires();
		EndImGuiFrame(); 
		renderer.SetSwapChain();
	}

	vertexConstantBufferInterface->Release();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


//	debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
//	debugInterface->Release();
	
	return 0;
}