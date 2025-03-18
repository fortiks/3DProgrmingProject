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
	//float4 position : SV_POSITION;
	float2 uv :	UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
	float3 positionWorld : WORLDPOSITION;
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
	//output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.colour = input.colour + colourModifier;
	output.uv = input.uv; //(input.uv.x, 1- input.uv.y);

	float4 worldPos = mul(float4(input.position, 1.0f), worldMatrix);
	output.positionWorld = worldPos.xyz;
	output.normal = mul(float4(input.normal, 0.0f), worldMatrix);

	return output;
}

//output.position = mul(float4(input.position + positionModifier, 1.0f), worldMatrix);
//output.position = float4(input.position + positionModifier, 1.0f);
