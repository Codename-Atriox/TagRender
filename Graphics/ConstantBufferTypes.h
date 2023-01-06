#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX wvpMatrix;
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMFLOAT3 camera_position;
};
//8 bytes -> 16

//struct CB_VS_pixelshader
//{
//	float alpha = 1.0f;
//};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientLightColor; //12
	float ambientLightStrenght; // 4
	// 16

	DirectX::XMFLOAT3 dynamicLightColor; //12
	float dynamicLightStrength; // 4
	// 16

	DirectX::XMFLOAT3 dynamicLightPosition; // 12
	float dynamicLightAttenuation_a;
	// 16

	DirectX::XMFLOAT3 directionalLightColor;
	float directionalLightStrength; //16
	DirectX::XMFLOAT3 directionalLightDirection;
	float dynamicLightAttenuation_b;
	// 16
	float dynamicLightAttenuation_c;
};



// ALL OF THESE HAVE TO BE 16 BYTE ALIGNED