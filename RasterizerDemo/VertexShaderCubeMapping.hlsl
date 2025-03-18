cbuffer CameraBuffer : register(b0)
{
	float4x4  viewProjectionMatrix;
};

cbuffer ObjectBuffer : register(b1)
{
	float4x4  worldMatrix;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float2 uv : UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
};

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float3 worldPos : WORLD_POS;
	float3 normal : NORMAL;
};

cbuffer VertexConstantBuffer : register(b2)
{
	float3 colourModifier;
	float padding;
	float3 positionModifier;
}



VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4x4  worldViewProj = mul(worldMatrix, viewProjectionMatrix);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Calculating world position
	float4 worldPos = mul(float4(input.position, 1.0f), worldMatrix);
	output.worldPos = worldPos.xyz;

	output.normal = mul(float4(input.normal, 0.0f), worldMatrix);

	return output;
}