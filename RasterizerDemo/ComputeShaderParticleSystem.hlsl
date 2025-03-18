struct Particle
{ 
    float3 position;
    float3 velocity;
    float lifetime;
    float size;
    float4 color;
};

RWStructuredBuffer<Particle> Particles : register(u0);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Particle gettingProcessed = Particles[DTid.x];
 // Logic manipulating "gettingProcessed" goes here
    float deltaTime = 0.016; // Assuming 60fps, deltaTime is about 1/60
    
    // Simulate fire rising & fading
    gettingProcessed.lifetime -= deltaTime; // Progress lifetime

    // Reset particle if it expires
    if (gettingProcessed.lifetime <= 0.0)
    {
        gettingProcessed.position = float3(15.0, 0.0, -1.0); // Reset position
        gettingProcessed.lifetime = 2.5;
        
    }
    else
    {
        gettingProcessed.position += gettingProcessed.velocity * deltaTime;
        
        gettingProcessed.color.rgb = lerp(float3(1, 0, 0), float3(1, 0.5, 0), 1.5f - gettingProcessed.lifetime);
        gettingProcessed.color.rgb = lerp(gettingProcessed.color.rgb, float3(1, 1, 0.3), 
            (1.0f - gettingProcessed.lifetime) * 0.8);

    }
    
    Particles[DTid.x] = gettingProcessed;
    
}