#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0){
    // object data
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
    float3 camera_position;
    // mesh data
    float3 pos_min;
    float3 pos_max;
    // UV data 
    float3 UV0_min;
    float3 UV0_max;
    float3 UV1_min;
    float3 UV1_max;
    float3 UV2_min;
    float3 UV2_max;
};


struct VS_I_POS{
    float3 inPos : POSITION;
};
struct VS_I_UV0{
    float2 inUV0 : TEXCOORD;
};
struct VS_I_UV1{
    float2 inUV1 : TEXCOORD;
};
struct VS_I_UV2{
    float2 inUV2 : TEXCOORD;
};
struct VS_I_COLOR{
    float4 inColor : COLOR;   // 8,8,8,8 (ARGB)
};
struct VS_I_NORMAL{
    float3    inNormal : NORMAL;  // note: is packed very weirdly (10,10,10,2)
};
struct VS_I_TANGENT{
    float3   inTangent : TANGENT; // 8,8,8,8 packed as 4byte (x,y,z, NULL?)
};

struct VS_OUTPUT{
    float4 outPosition : SV_POSITION;
    float2 outUV0 : TEXCOORD0;
    float2 outUV1 : TEXCOORD1;
    float2 outUV2 : TEXCOORD2;
    float4 outColor : COLOR;
    float3 outNormal : NORMAL;
    float3 outTangent : TANGENT; // im pretty sure this isn't actually used in the pixel shader?
    // other data stuff
    float3 outWorldPos : WORLD_POSITION;
    float3 outCamDirection : CAM_DIRECTION;
};

VS_OUTPUT main(VS_I_POS pos, VS_I_UV0 uv0, VS_I_UV1 uv1, VS_I_UV2 uv2, VS_I_COLOR color, VS_I_NORMAL normal, VS_I_TANGENT tangent)
{
    VS_OUTPUT output;
    
    output.outPosition = mul(float4(pos.inPos, 1.0f), wvpMatrix);
    output.outUV0 = uv0.inUV0;
    output.outUV1 = uv1.inUV1;
    output.outUV2 = uv2.inUV2;
    
    output.outColor = color.inColor;
    output.outNormal = normalize(mul(float4(normal.inNormal, 0.0f), worldMatrix));
    output.outTangent = tangent.inTangent;
    
    output.outWorldPos = mul(float4(pos.inPos, 1.0f), worldMatrix);
    output.outCamDirection = normalize(output.outPosition.xyz - camera_position);
    return output;
}