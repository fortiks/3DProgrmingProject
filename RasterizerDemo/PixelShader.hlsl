Texture2D ambientTexture : register(t0); // Ka
Texture2D diffuseTexture : register(t1); // Kd
Texture2D specularTexture : register(t2); // Ks
SamplerState  Sampler : register(s0);

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
    float4 textureColor : SV_Target1; // RGBA: (Kd)
    float4 normal : SV_Target2; // RGB: Normal, 
    float4 ambientSpecular : SV_Target3; // RGB : Ambient intensity (Ka) A: Specular intensity (Ks)
};

cbuffer renderMode : register(b1)
{
    int renderType;
};

PSOutput main(PixelShaderInput input)
{
	// Texture Color
	PSOutput output;
    
    
    float4 ambientSample = ambientTexture.Sample(Sampler, input.uv);
    float4 diffuseSample = diffuseTexture.Sample(Sampler, input.uv);
    float4 specularSample = specularTexture.Sample(Sampler, input.uv);
    
    if (renderType == 0)
    {
        output.textureColor = diffuseSample;
        output.ambientSpecular = float4(specularSample.rgb, ambientSample.r);
        output.normal = float4(input.normal, 0);
        
    }
    else if (renderType == 1)
    {
        output.textureColor = float4(input.colour, 1);
        output.normal = float4(input.normal, 0);
        
        output.ambientSpecular = float4(0.25, 0.25, 0.25, 0.25);

    }
    else
    {
        output.textureColor = float4(0, 0, 0, 1); // Default black
        output.normal = float4(input.normal, 0);
        output.ambientSpecular = float4(0.25, 0.25, 0.25, 0.25);
    }
    
	
	output.position = float4(input.positionWorld, 1);
	return output;
}