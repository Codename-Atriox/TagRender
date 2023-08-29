#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
using namespace DirectX;

class Camera : public GameObject
{
public:
	Camera();
	void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	const XMMATRIX& GetViewMatrix() const;
	const XMMATRIX& GetProjectionMatrix() const;

	float speed = 0.1f;
private:
	void UpdateMatrix() override;

	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};