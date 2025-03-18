cbuffer CameraBuffer : register(b0)
{
	float4x4  viewProjectionMatrix;
};

Texture2D displacementMap : register(t0); 
SamplerState samplerState : register(s0); 

struct DomainShaderOutput
{
	float4 position : SV_POSITION;
	float2 uv :	UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
	float3 positionWorld : WORLDPOSITION;
};

struct HullShaderOutput
{
	//float4 position : SV_POSITION;
	float2 uv :	UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
	float3 positionWorld : WORLDPOSITION;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DomainShaderOutput main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 barycentric : SV_DomainLocation,
	const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch)
{
	DomainShaderOutput output;

	float3 tessellatedPosition = patch[0].positionWorld * barycentric.x + patch[1].positionWorld * barycentric.y +
		patch[2].positionWorld * barycentric.z;

	
	output.normal = normalize(patch[0].normal * barycentric.x + patch[1].normal * barycentric.y +
		patch[2].normal * barycentric.z);

	output.uv = patch[0].uv * barycentric.x +
			patch[1].uv * barycentric.y + patch[2].uv * barycentric.z;

	output.colour = patch[0].colour * barycentric.x +
		patch[1].colour * barycentric.y + patch[2].colour * barycentric.z;


	// Displacement test: Move vertices along normal
	float displacementAmount = 0.01;

	tessellatedPosition += output.normal * displacementAmount;

	// Displacement map test: Move vertices along texture

	float displacement = displacementMap.SampleLevel(samplerState, output.uv, 0).r; // Use red channel

	//tessellatedPosition += output.normal * displacement;

	output.positionWorld = tessellatedPosition;

	output.position = mul(float4(output.positionWorld, 1), viewProjectionMatrix);

	return output;
}
