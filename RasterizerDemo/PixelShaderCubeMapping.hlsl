// Environment/cube Mapping Pixel Shader

TextureCube cubeMap : register(t0);  // Cube Map Texture
SamplerState samplerState : register(s0);   // Sampler

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 worldPos : WORLD_POS;
	float3 normal : NORMAL;
};;

cbuffer CameraPosBuffer : register(b0)
{
	float3 cameraPosition;
};


float4 main(PixelShaderInput input) : SV_Target
{
	// Texture Color
	// compute normal in world space
	float3 normal = normalize(input.normal);
	
// Compute view direction 
    float3 viewDir = normalize(input.worldPos - cameraPosition);

	// Compute reflection vector
	float3 ReflectionVector = reflect(viewDir, normal);

	float4 sampleValue = cubeMap.Sample(samplerState, ReflectionVector);
    return sampleValue;

}