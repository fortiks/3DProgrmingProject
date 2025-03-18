struct PixelShaderInput
{
    float4 position : SV_Position;
    float3 velocity : VELOCITY;
    float size : SIZE;
    float lifetime : LifeTime;
    float4 color : COLOR;
};

float4 main(PixelShaderInput input) : SV_Target
{
    return input.color;
}