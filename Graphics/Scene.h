#pragma once
#include "../Slipspace/UI/UI_Manager.h"

class Scene {
public:
	//RenderableGameObject gameObject;
	//Light light;
	bool Init(HWND hwnd, int width, int height);
	void RenderFrame();
	Camera camera;
private:
	ModuleManager Modules;
	UI ui;
	Graphics gfx; // so it can be accessed by

	Timer fpsTimer;
	int fpsCoutner = 0;
	std::string fpsString = "FPS: null";

};