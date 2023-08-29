#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0){
    // object data
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
    float3 camera_position;
    // mesh data
    float3 minbounds;
    float3 maxbounds;
    // UV data 
};


struct VS_INPUT{
    min16uint4 inPos : POSITION;
    min16uint2 inUV0 : TEXCOORD0;
    min16uint2 inUV1 : TEXCOORD1;
    min16uint2 inUV2 : TEXCOORD2;
    min16uint4 inColor : COLOR;
    uint inNormal : NORMAL; // note: is packed very weirdly
     
    /* 
	*  0 : Position (wordVector4DNormalized)
	*  1 : UV0 (wordVector2DNormalized)
	*  2 : UV1 (wordVector2DNormalized)
	*  3 : UV2 (wordVector2DNormalized)
	*  4 : Color (byteARGBColor)
	*  5 : Normal (_10_10_10_2_signedNormalizedPackedAsUnorm)
	*  6 : Tangent (byteUnitVector3D)
    */
};
struct VS_OUTPUT{
    float4 outPosition : SV_POSITION;
    float2 outUV0 : TEXCOORD0;
    float2 outUV1 : TEXCOORD1;
    float2 outUV2 : TEXCOORD2;
    float3 outColor : COLOR;
    float3 outNormal : NORMAL;
    // other data stuff
    float3 outWorldPos : WORLD_POSITION;
    float3 outCamDirection : CAM_DIRECTION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    // fixup geo position
    float3 decompressed_position = float3(input.inPos.xyz) / float(0xffff); // positions are normalized, meaning they are between 0.0 & 1.0
    decompressed_position *= maxbounds - minbounds;
    decompressed_position += minbounds;
    
    float3 decompressed_UV0 = float3(0, 0, 0);
    float3 decompressed_UV1 = float3(0, 0, 0);
    float3 decompressed_UV2 = float3(0, 0, 0);
    
    float3 decompressed_normal = float3(0, 0, 0);
    
    
    output.outPosition = mul(float4(decompressed_position, 1.0f), wvpMatrix);
    output.outCamDirection = normalize(output.outPosition.xyz - camera_position);
    output.outUV0 = decompressed_UV0;
    output.outUV0 = decompressed_UV1;
    output.outUV0 = decompressed_UV2;
    output.outNormal = normalize(mul(float4(decompressed_normal, 0.0f), worldMatrix));
    output.outWorldPos = mul(float4(decompressed_position, 1.0f), worldMatrix);
    return output;
}