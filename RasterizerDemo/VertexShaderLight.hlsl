#pragma once
cbuffer CameraBuffer : register(b0)
{
    float4x4  viewProjectionMatrix;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4  worldMatrix;
};

struct VSInput
{
    float3 position : POSITION;  // Model space position
};

struct VSOutput
{
    float4 position : SV_POSITION;  // Position in light clip space
};

// Vertex Shader for Depth Pass
VSOutput VS_DepthPass(VSInput input)
{
    VSOutput output;

    // Transform vertex from model space -> world space -> light clip space
    output.position = mul(float4(input.position, 1.0f), worldMatrix);
    output.position = mul(output.position, lightViewProj);

    return output;
}