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
    min16uint4 inPos : POSITION;
};
struct VS_I_UV0{
    min16uint2 inUV0 : TEXCOORD;
};
struct VS_I_UV1{
    min16uint2 inUV1 : TEXCOORD;
};
struct VS_I_UV2{
    min16uint2 inUV2 : TEXCOORD;
};
struct VS_I_COLOR{
    uint inColor : COLOR;   // 8,8,8,8 (ARGB)
};
struct VS_I_NORMAL{
    uint    inNormal : NORMAL;  // note: is packed very weirdly (10,10,10,2)
};
struct VS_I_TANGENT{
    uint   inTangent : TANGENT; // 8,8,8,8 packed as 4byte (x,y,z, NULL?)
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
    // fixup geo position
    float3 decompressed_position = float3(pos.inPos.xyz) / float(0xffff); // positions are normalized, meaning they are between 0.0 & 1.0
    decompressed_position *= 1 - (-1); // apparently we dont use the compressed position regardless?
    decompressed_position += (-1);
    //decompressed_position *= pos_max - pos_min;
    //decompressed_position += pos_min;
    
    float2 decompressed_UV0 = float3(0, 0, 0);
    float2 decompressed_UV1 = float3(0, 0, 0);
    float2 decompressed_UV2 = float3(0, 0, 0);
    
    float4 decompressed_color = float4(((color.inColor >> 16) & 0xff) / 256.0, ((color.inColor >> 8) & 0xff) / 256.0, (color.inColor & 0xff) / 256.0, (color.inColor >> 24) / 256.0);
    
    float norm_x = (((normal.inNormal >> 20) & 0x3ff) / 511.0) - 1.0;
    float norm_y = (((normal.inNormal >> 10) & 0x3ff) / 511.0) - 1.0;
    float norm_z = ((normal.inNormal & 0x3ff) / 511.0) - 1.0;
    
    float3 decompressed_normal = float3(norm_x, norm_y, norm_z);
    float3 decompressed_tangent = float3(0, 0, 0);
    
    
    
    output.outPosition = mul(float4(decompressed_position, 1.0f), wvpMatrix);
    output.outUV0 = decompressed_UV0;
    output.outUV1 = decompressed_UV1;
    output.outUV2 = decompressed_UV2;
    
    output.outColor = decompressed_color;
    output.outNormal = normalize(mul(float4(decompressed_normal, 0.0f), worldMatrix));
    output.outTangent = decompressed_tangent;
    
    output.outWorldPos = mul(float4(decompressed_position, 1.0f), worldMatrix);
    output.outCamDirection = normalize(output.outPosition.xyz - camera_position);
    return output;
}