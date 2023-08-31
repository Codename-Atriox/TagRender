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
    float2 inUV0 : TEXCOORD0;
    float2 inUV1 : TEXCOORD1;
    float2 inUV2 : TEXCOORD2;
    float4 inColor : COLOR;
    float3 inNormal : NORMAL;
    float3 inTangent : TANGENT; // may not be of use in here
    // other data stuff
    float3 inWorldPos : WORLD_POSITION;
    float3 inCamDirection : CAM_DIRECTION;

};

//Texture2D objTexture : TEXTURE : register(t0);
//Texture2D objNormal : TEXT_NORMAL : register(t1);
//SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // LOAD TEXTURE COORDS
    //float3 sampleColor = objTexture.Sample(objSamplerState, input.inUV0);
    float3 sampleColor = input.inWorldPos;
    //float3 sampleColor = input.inNormal; // 
    //sampleColor = float3(0.5, 0.5, 0.5);
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
    
    
    
    float3 finalColor = sampleColor; // * (ambientLight + directionalLight + specularLight);
    return float4(finalColor, 1.0f); // we can add in alpha support once we make sure everything else works fine
}