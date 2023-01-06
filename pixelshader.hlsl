//cbuffer alphaBuffer : register(b0)
//{
//    float alpha;
//}

cbuffer lightBuffer : register(b0)
{
    // AMBIENT LIGHT
    float3 ambientLightcolor;
    float ambientLightStrength;
    // DYNAMIC LIGHT
    float3 dynamicLightColor;
    float dynamicLightStrength;
    float3 dynamicLightPosition;
    float dynamicLightAttenuation_a;
    
    // DIRECTIONAL LIGHT
    float3 directionalLightColor;
    float directionalLightStrength;
    float3 directionalLightDirection;
    
    float dynamicLightAttenuation_b;
    float dynamicLightAttenuation_c;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal : NORMAL;
    float3 inWorldPos : WORLD_POSITION;
    float3 inCamDirection : CAM_DIRECTION;
};

Texture2D objTexture : TEXTURE : register(t0);
Texture2D objNormal : TEXT_NORMAL : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // LOAD TEXTURE COORDS
    float3 sampleColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    //float3 sampleColor = input.inNormal; // test normals
    
    // AMBIENT LIGHTING
    float3 ambientLight = ambientLightcolor * ambientLightStrength;
    
    // DIRECTIONAL LIGHTING
    float directionalLightFactor = max(0.0, dot(directionalLightDirection, input.inNormal));
    float3 directionalLight = directionalLightStrength * directionalLightFactor * directionalLightColor;
    
    // SPECULAR LIGHT 
    //float SpecularStrength = 1.0;
    //float3 SpecularColor = float3(1.0f, 1.0f, 1.0f);
    float3 reflected_light = reflect(directionalLightDirection, input.inNormal);
    float specularity = 30.0;
    float SpecularityAmount = pow(max(0.0f, dot(reflected_light, input.inCamDirection)), specularity);
    float3 specularLight = directionalLightStrength * SpecularityAmount * directionalLightColor;
    // specularity shouldn't include color
    
    
    // DYNAMIC LIGHTING
    //float3 vectorToLight = normalize(dynamicLightPosition - input.inWorldPos); 
    //float3 diffuseLightIntensity = max(dot(vectorToLight, input.inNormal), 0);
    //float distanceToLight = distance(dynamicLightPosition, input.inWorldPos);
    //float attenuationFactor = 1 / (dynamicLightAttenuation_b + dynamicLightAttenuation_b * distanceToLight + dynamicLightAttenuation_c * pow(distanceToLight, 2));
    //diffuseLightIntensity *= attenuationFactor;
    //float3 diffuseLight = diffuseLightIntensity * dynamicLightStrength * dynamicLightcolor;
    //float3 appliedLight = ambientLight;
    //appliedLight += diffuseLight;
    
    
    
    float3 finalColor = sampleColor * (ambientLight + directionalLight + specularLight);
    return float4(finalColor, 1.0f);
}