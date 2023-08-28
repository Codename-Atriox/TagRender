#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0){
    // object data
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
    float3 camera_position;
    // mesh data // float4 for maths convenience
    float3 minbounds;
    float3 maxbounds;
};


struct VS_INPUT{
    min16uint4 inPos : POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal  : NORMAL;
};
struct VS_OUTPUT{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
    float3 outNormal : NORMAL;
    float3 outWorldPos : WORLD_POSITION;
    float3 outCamDirection : CAM_DIRECTION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    // fixup geo position
    float3 decompressed_position = input.inPos.xyz / float(0xffff); // positions are normalized, meaning they are between 0.0 & 1.0
    decompressed_position *= maxbounds - minbounds;
    decompressed_position += minbounds;
    
    output.outPosition = mul(float4(decompressed_position, 1.0f), wvpMatrix);
    output.outCamDirection = normalize(output.outPosition.xyz - camera_position);
    output.outTexCoord = input.inTexCoord;
    output.outNormal = normalize(mul(float4(input.inNormal, 0.0f), worldMatrix));
    output.outWorldPos = mul(float4(decompressed_position, 1.0f), worldMatrix);
    return output;
}