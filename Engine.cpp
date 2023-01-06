#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	return true;
}

bool Engine::ProcessMessages() 
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float DeltaTime = timer.GetMilisecondsElapsed();
	timer.Restart();

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}
	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	// translate mouse movement to camera rotation
	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->gfx.camera.AdjustRotation(
					(float)me.GetPosY() * 0.01f,
					(float)me.GetPosX() * 0.01f,
					0);
			}
		}
	}

	this->gfx.gameObject.AdjustRotation(0.0f, 0.001f*DeltaTime, 0.0f);

	 // translate key inputs to camera movement
	float cameraSpeed = 0.008f;
	if (keyboard.IsKeyPressed(VK_SHIFT))
	{
		cameraSpeed = 0.4f;
	}

	float deltaSpeed = cameraSpeed * DeltaTime;

	if (keyboard.IsKeyPressed('W'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * deltaSpeed);
	}
	if (keyboard.IsKeyPressed('S'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * deltaSpeed);
	}
	if (keyboard.IsKeyPressed('A'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * deltaSpeed);
	}
	if (keyboard.IsKeyPressed('D'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * deltaSpeed);
	}

	if (keyboard.IsKeyPressed(VK_SPACE))
	{
		this->gfx.camera.AdjustPosition(0.0f, deltaSpeed, 0.0f);
	}
	if (keyboard.IsKeyPressed('Z'))
	{
		this->gfx.camera.AdjustPosition(0.0f, -deltaSpeed, 0.0f);
	}


	if (keyboard.IsKeyPressed('C'))
	{
		XMVECTOR lightPosition = this->gfx.camera.GetPositionVector();
		lightPosition += this->gfx.camera.GetForwardVector();
		this->gfx.light.SetPosition(lightPosition);
		this->gfx.light.SetRotation(this->gfx.camera.GetRotationFloat3());
	}
}

void Engine::RenderFrame()
{
	gfx.RenderFrame();
}
