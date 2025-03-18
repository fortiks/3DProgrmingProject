RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);


// ==== Spot Light ===
struct SpotLight
{
	float4x4 vpMatrix;
	float3 colour;
	float3 direction;
	float angle;
	float3 position;
};

StructuredBuffer<SpotLight> SpotLights : register(t3);
Texture2DArray<float> shadowMaps : register(t4);
sampler shadowMapSampler : register(s0);

 // ===  Directional Light ===

StructuredBuffer<SpotLight> DirectionalLights : register(t6);
Texture2DArray<float> shadowMapsDir : register(t7);
sampler shadowMapSamplerDirectional : register(s1);


struct IMGuiVariables
{
    int isShadow;
    int3 padding;
};

StructuredBuffer<IMGuiVariables> IMGuiControls : register(t5);


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

	float3 position = positionGBuffer[DTid.xy].xyz;
	float3 colour = colorGBuffer[DTid.xy].xyz;
	float3 normal = normalize(normalGBuffer[DTid.xy].xyz);
	float4 backgroundColor = float4(0.169, 0.18, 0.98, 1); // Gray background
    float4 testColor = colorGBuffer[DTid.xy];


     // Assume camera is at origin (0,0,0)
    float3 viewDir = normalize(-position);

    // Final color
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    float shadowMapDepth;
    float3 shadowMapUV;
    float lightDepth;
    float shadowFactor;
    // Loop through all spotlights

    uint lightCount = 3;
    for (uint i = 0; i < lightCount; i++)
    {
        SpotLight light = SpotLights[i];

        // Calculate light direction and spotlight
        float3 lightDir = normalize(light.position - position);
        float spotFactor = dot(-normalize(light.direction), lightDir);
        spotFactor = smoothstep(cos(light.angle), 1.0, spotFactor); 

        // === Shadow Mapping ===
        // Transform world position to light space
       
        float4 lightSpacePos = mul(float4(position, 1.0), light.vpMatrix); 
        lightSpacePos.xyz /= lightSpacePos.w; // Perspective divide // turn into ndc coordinates
        //   shadow UV
        shadowMapUV = float3(lightSpacePos.x * 0.5 + 0.5, lightSpacePos.y * -0.5 + 0.5f, i);

        shadowMapDepth = shadowMaps.SampleLevel(shadowMapSamplerDirectional, shadowMapUV, 0);
        //shadowMapDepth = pow((shadowMapDepth - 0.01) / (100 - 0.01), 1.5);
        lightDepth = lightSpacePos.z;
        bool inShadow = lightDepth > shadowMapDepth + 0.01f; // Add bias to avoid self-shadowing

        // Apply shadow factor
        shadowFactor = inShadow ? 0.0f : 1.0f;


        // Calculate diffuse lighting
        float diff = max(dot(normal, lightDir), 0.0);

        // Blinn-Phong Specular Lighting
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 50.0); // Shininess = 50.0

        float Intensity = 2.5;
        // Combine components

        float3 lightEffect = (diff + spec) * light.colour * spotFactor * Intensity;
        if (IMGuiControls[0].isShadow)
        {
            lightEffect = (diff + spec) * light.colour * spotFactor * Intensity * shadowFactor;
        }
        

        // Accumulate lighting
        finalColor += lightEffect;
    }
    
    
    uint directionalLightCount = 1;
     // Loop through all Directional Ligts
    for (uint i = 0; i < directionalLightCount; i++)
    {
        SpotLight light = DirectionalLights[i];
        float3 lightDir = normalize(-light.direction); // Directional light has a direction but no position
        
        // === Shadow Mapping ===
        float4 lightSpacePos = mul(float4(position, 1.0), light.vpMatrix); // Transformation to light space
        lightSpacePos.xyz /= lightSpacePos.w; // Perspective divide (if needed for your setup)
    
        shadowMapUV = float3(lightSpacePos.x * 0.5 + 0.5, lightSpacePos.y * -0.5 + 0.5f, i); // Shadow map UV coordinates
        shadowMapDepth = shadowMapsDir.SampleLevel(shadowMapSamplerDirectional, shadowMapUV, 0);
        lightDepth = lightSpacePos.z;
        bool inShadow = lightDepth > shadowMapDepth + 0.01f; // Bias to avoid self-shadowing
        float shadowFactor = inShadow ? 0.0f : 1.0f;
        
        // Calculate diffuse lighting
        float diff = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong Specular Lighting
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 50.0);

        float Intensity = 1;
        float3 lightEffect = (diff + spec) * light.colour * Intensity;

        if (IMGuiControls[0].isShadow)
        {
            lightEffect *= shadowFactor;
        }

        finalColor += lightEffect;
    }
    
    float3 ambient = colour * 0.25f;
    // Apply base color and write output
    backBufferUAV[DTid.xy] = (testColor.a > 0.0) ? float4(ambient + colour * finalColor, 1.0) : backgroundColor;
   //float4(shadowMapDepth, shadowMapDepth, shadowMapDepth, 1.0f); //
}

/*// Diffuse
input.normal = normalize(input.normal);
float3 lightPixelVector = normalize(Lightposition - input.positionWorld);
float diffuseConst = max(dot(lightPixelVector, input.normal.xyz), 0);

float3 diffuse = (LightColour * diffuseConst) * LightIntensity;

// Ambient light
float3 ambient = LightColour * 0.5f;

// Reflection
float3 reflection = normalize(reflect(-lightPixelVector, input.normal.xyz));

// Specular
float3 Viewvector = normalize(Cameraposition - input.positionWorld);
float specularConst = pow(max(dot(reflection, Viewvector), 0), 50.f);
float3 specular = dot(LightColour, specularConst);

// Final Colour
float4 finalColour = (float4(diffuse, 1.0f) + float4(ambient, 1.0f)) * mat + float4(specular, 1.0f);*/
