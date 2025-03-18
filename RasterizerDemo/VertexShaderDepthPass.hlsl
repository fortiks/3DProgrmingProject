#pragma once
cbuffer CameraBuffer : register(b0)
{
    float4x4  viewProjectionMatrix;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4  worldMatrix;
};

cbuffer VertexConstantBuffer : register(b2)
{
    float3 colourModifier;
    float padding;
    float3 positionModifier;
}

struct VSInput
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : Normal;
    float3 colour : COLOUR;
};

struct VSOutput
{
    float4 position : SV_POSITION;  // Position in light clip space
};

// Vertex Shader for Depth Pass
VSOutput main(VSInput input)
{
    VSOutput output;

    // Transform vertex from model space -> world space -> light clip space
    float4 worldPos = mul(float4(input.position, 1.0f), worldMatrix);
    output.position = mul(worldPos, viewProjectionMatrix);
    //output.position = worldPos;
    return output;
}