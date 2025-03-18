cbuffer TessellationBuffer : register(b0)
{
	float4 CameraPosition; // Camera position in world space
	float TessMax;         // Maximum tessellation factor (near)
	float TessMin;         // Minimum tessellation factor (far)
	float DecayFactor;     // Controls how quickly tessellation decreases
}

#define MIN_DISTANCE 0.5f
#define MAX_DISTANCE 100.0f

struct VertexShaderOutput
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;


	// Compute center (average of control points)
	float3 patchCenter = (ip[0].positionWorld.xyz +
		ip[1].positionWorld.xyz +
		ip[2].positionWorld.xyz) / 3.0f;

	// Compute distance from the camera
	float distance = length(CameraPosition.xyz - patchCenter);

	// Remap distance to tessellation factor
	float tessFactor = lerp(TessMax, TessMin, saturate((distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE))); // clamp distance between 0 -1


	Output.EdgeTessFactor[0] =
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.InsideTessFactor = tessFactor;

	return Output;
}


struct HullShaderOutput
{
	//float4 position : SV_POSITION;
	float2 uv :	UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
	float3 positionWorld : WORLDPOSITION;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HullShaderOutput main(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID)
{
	HullShaderOutput Output;

	//Output.position = ip[i].position;
	Output.uv = ip[i].uv;
	Output.normal = ip[i].normal;
	Output.colour = ip[i].colour;
	Output.positionWorld = ip[i].positionWorld;

	return Output;
}
