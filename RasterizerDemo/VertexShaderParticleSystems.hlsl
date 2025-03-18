cbuffer Camera : register(b0)
{
    float4x4 vp;
    float3 cameraPosition;
};


struct Particle
{
    float3 position;
    float3 velocity;
    float lifetime;
    float size;
    float4 color;
};


struct VertexShaderOutput
{
	float3 position : POSITION;
    float3 velocity : VELOCITY;
    float size : SIZE;
    float lifetime : LIFETIME;
    float4 color : COLOR;
};

StructuredBuffer<Particle> Particles : register(t0);

VertexShaderOutput main(uint vertexID : SV_VertexID)
{    
    VertexShaderOutput output;
    Particle p = Particles[vertexID];
    
    output.position = p.position;
    output.velocity = p.velocity;
    output.size = p.size;
    output.lifetime = p.lifetime;
    output.color = p.color;
    
    return output;
}
