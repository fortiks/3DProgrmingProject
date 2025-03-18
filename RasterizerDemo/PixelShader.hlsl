Texture2D testTexture : register(t0);
SamplerState  testSampler : register(s0);

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 uv : UV;
	float3 normal : Normal;
	float3 colour : COLOUR;
	float3 positionWorld : WORLDPOSITION;
};

// G-Buffer Outputs
struct PSOutput {
	float4 position  : SV_Target0;
	float4 textureColor	: SV_Target1;
	float4 normal    : SV_Target2;	
};

cbuffer renderMode : register(b0)
{
    float renderType;
};

PSOutput main(PixelShaderInput input)
{
	// Texture Color
	PSOutput output;
	
    if (renderType == 0)
    {
        float4 mat = testTexture.Sample(testSampler, input.uv);
        clip(mat.w - 0.1);
        output.textureColor = mat;
    }
    else
    {
        output.textureColor = float4(input.colour, 1.0f);

    }
        
	output.normal = float4(input.normal, 0);
	output.position = float4(input.positionWorld, 1);
	return output;
}