cbuffer Camera : register(b0)
{
    float4x4 vp;
    float3 cameraPosition;
};

struct GeometryShaderOutput
{
    float4 position : SV_Position;
    float3 velocity : VELOCITY;
    float size : SIZE;
    float lifetime : LIFETIME;
    float4 color : COLOR;
};

struct GeometryShaderInput
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float size : SIZE;
    float lifetime : LIFETIME;
    float4 color : COLOR;
};

[maxvertexcount(6)]
void main(
point GeometryShaderInput input[1],
inout TriangleStream<GeometryShaderOutput> output)
{
    
    // Calculate front vector using input and camera position (particle -> camera) 
    float3 position = input[0].position;
    
    float3 frontVec = normalize(cameraPosition - position);
    
    // Use front vector along with defaulted up vector to calculate right vector
    float3 upVec = float3(0, 1, 0); // defaulted
    float3 rightVec = normalize(cross(upVec, frontVec)); // Right perpendicular to the coordinates  
    
    // Use front vector along with right vector to calculate actual up vector
    upVec = cross(frontVec, rightVec);
    
    // Scale right and up at end based on preference
    rightVec *= input[0].size;
    upVec *= input[0].size;
    
    // vertexes Quad 
    float3 v0 = position - rightVec + upVec; // top Left
    float3 v1 = position + rightVec + upVec; // top Right
    float3 v2 = position - rightVec - upVec; // bottom Left
    float3 v3 = position + rightVec - upVec; // bottom Right
    
    
    GeometryShaderOutput toAppend;
    toAppend.velocity = input[0].velocity;
    toAppend.lifetime = input[0].lifetime;
    toAppend.size = input[0].size;
    toAppend.color = input[0].color;
    
    
    toAppend.position = mul(float4(v0, 1.0f), vp); // Top left
     
    output.Append(toAppend);    // omg you need to get this brain dead shit in correct order aka conter clockwise otherwise this stupid shit dont show ffffuuuuuuuuckk
     
    toAppend.position = mul(float4(v2, 1.0f), vp); // bottom right
   
    output.Append(toAppend);
    
    toAppend.position = mul(float4(v1, 1.0f), vp); // bottom left

    output.Append(toAppend);
    
    output.RestartStrip(); // Done with first triangle
    
    toAppend.position = mul(float4(v1, 1.0f), vp); // Top left
     
    output.Append(toAppend); 
     
    toAppend.position = mul(float4(v2, 1.0f), vp); // bottom right
   
    output.Append(toAppend);
    
    toAppend.position = mul(float4(v3, 1.0f), vp); // bottom left

    output.Append(toAppend);

}
