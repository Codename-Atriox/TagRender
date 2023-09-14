
#include "Scene.h"

bool Scene::Init(HWND hwnd, int width, int height) {
	fpsTimer.Start();
	error_display_timer.Start();

	ui.init(&gfx, &Modules);

	if (!gfx.Initialize(hwnd, width, height))
		return false;
	Modules.gfx = &gfx;

	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(
		90.0f,
		static_cast<float>(width) / static_cast<float>(height),
		0.1f,
		300000.0f);

	return true;
}

void Scene::RenderFrame()
{

	//gfx.deviceContext->OMSetRenderTargets(1, gfx. &renderTargetViewMap, NULL);
	gfx.deviceContext->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), gfx.depthStencilView.Get());

	gfx.cb_vs_vertexshader.data.camera_position = camera.GetPositionFloat3();
	gfx.cb_vs_generic_vertexshader.data.camera_position = camera.GetPositionFloat3();


	// SETUP LIGHTING
	//gfx.cb_ps_light.data.dynamicLightColor = light.lightColor;
	//gfx.cb_ps_light.data.dynamicLightStrength = light.lightStrength;
	//gfx.cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	//gfx.cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	//gfx.cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	//gfx.cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;

	gfx.cb_ps_light.ApplyChanges();
	gfx.deviceContext->PSSetConstantBuffers(0, 1, gfx.cb_ps_light.GetAddressOf());



	// setup shared shader stuff that will be used to render UI models aswell as scene
	gfx.deviceContext->IASetInputLayout(gfx.generic_vertexshader.GetInputLayout());
	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.deviceContext->RSSetState(gfx.rasterizerState.Get());
	gfx.deviceContext->OMSetDepthStencilState(gfx.depthStencilState.Get(), 0);
	gfx.deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	//gfx.deviceContext->OMSetBlendState(gfx.blendState.Get(), NULL, 0xFFFFFFFF);
	gfx.deviceContext->PSSetSamplers(0, 1, gfx.samplerState.GetAddressOf());
	gfx.deviceContext->VSSetShader(gfx.generic_vertexshader.GetShader(), NULL, 0);
	gfx.deviceContext->PSSetShader(gfx.generic_pixelshader.GetShader(), NULL, 0);



	// setup background stuff
	float bgcolor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
	gfx.deviceContext->ClearRenderTargetView(gfx.renderTargetView.Get(), bgcolor);
	gfx.deviceContext->ClearDepthStencilView(gfx.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// render a test element
	Vertex v[] ={
		Vertex(-0.5f,-0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,0.0f), // FRONT bottom left
		Vertex(-0.5f, 0.5f,-0.5f,  0.0f,0.0f,  0.0f,0.0f,0.0f), // FRONT top left
		Vertex(0.5f, 0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,0.0f), // FRONT top right
		Vertex(0.5f,-0.5f,-0.5f,  1.0f,1.0f,  0.0f,0.0f,0.0f), // FRONT bottom right

		Vertex(-0.5f,-0.5f, 0.5f,  0.0f,1.0f,  0.0f,0.0f,0.0f), // BACK bottom left
		Vertex(-0.5f, 0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,0.0f), // BACK top left
		Vertex(0.5f, 0.5f, 0.5f,  1.0f,0.0f,  0.0f,0.0f,0.0f), // BACK top right
		Vertex(0.5f,-0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,0.0f), // BACK bottom right
	};

	VertexBuffer<Vertex> test_vert_buffer = {};
	// LOAD VERTEX DATA
	HRESULT hr = test_vert_buffer.Initialize(gfx.device.Get(), v, ARRAYSIZE(v));   // gfx.device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, gfx.vertexBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create vertex buffer");

	DWORD indicies[] ={
		0, 1, 2, // front 
		0, 2, 3, // front
		4, 7, 6, // back
		4, 6, 5, // back
		3, 2, 6, // right
		3, 6, 7, // right
		4, 5, 1, // left
		4, 1, 0, // left
		1, 5, 6, // top
		1, 6, 2, // top
		0, 3, 7, // bottom
		0, 7, 4  // bottom
	};

	// LOAD INDEX DATA
	IndexBuffer test_index_buffer = {};
	hr = test_index_buffer.Initialize(gfx.device.Get(), indicies, ARRAYSIZE(indicies)); // device->CreateBuffer(&indexBufferDesc, &indexBufferData, indiciesBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create indicies buffer");

	// render test data
	gfx.deviceContext->VSSetConstantBuffers(0, 1, gfx.cb_vs_generic_vertexshader.GetAddressOf());


	XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(0, 0, 0);

	gfx.deviceContext->VSSetConstantBuffers(0, 1, gfx.cb_vs_generic_vertexshader.GetAddressOf());
	gfx.cb_vs_generic_vertexshader.data.wvpMatrix = DirectX::XMMatrixIdentity() * worldMatrix * (camera.GetViewMatrix() * camera.GetProjectionMatrix());
	gfx.cb_vs_generic_vertexshader.data.worldMatrix = DirectX::XMMatrixIdentity() * worldMatrix;
	gfx.cb_vs_generic_vertexshader.ApplyChanges();



	UINT offset = 0;
	gfx.deviceContext->IASetVertexBuffers(0, 1, test_vert_buffer.GetAddressOf(), test_vert_buffer.StridePtr(), &offset);
	gfx.deviceContext->IASetIndexBuffer(test_index_buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	gfx.deviceContext->DrawIndexed(test_index_buffer.IndexCount(), 0, 0);
	// end test section

	gfx.deviceContext->IASetInputLayout(gfx.vertexshader.GetInputLayout());
	gfx.deviceContext->VSSetShader(gfx.vertexshader.GetShader(), NULL, 0);
	gfx.deviceContext->PSSetShader(gfx.pixelshader.GetShader(), NULL, 0);

	// render loaded bsp if any
	if (ui.active_bsp != nullptr)
		RenderBSP::Render(ui.active_bsp, &gfx, camera.GetViewMatrix() * camera.GetProjectionMatrix(), camera.GetPositionFloat3());

	//static float alpha = 0.5f;
	{ // 

		//gfx.gameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());

		// PIXEL SHADER
		//gfx.cb_vs_pixelshader.data.alpha = 1.0f;
		//gfx.cb_vs_pixelshader.ApplyChanges();
		//gfx.deviceContext->PSSetConstantBuffers(0, 1, gfx.cb_vs_pixelshader.GetAddressOf());

	}
	{

		//gfx.deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		//gfx.light.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());

	}

	// testing junk
	//ui.render_GEO(&Modules, device.Get(), deviceContext.Get(), &cb_vs_vertexshader, camera.GetViewMatrix() * camera.GetProjectionMatrix()); // call to handle our Slipspace interface UI





	// SETUP FPS COUNTER
	fpsCoutner += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0){
		fpsString = "FPS: " + std::to_string(fpsCoutner);
		fpsCoutner = 0;
		fpsTimer.Restart();
	}
	const int errors_onscreen_at_once = 10;
	if (error_display_timer.GetMilisecondsElapsed() > 3000.0){
		if (ErrorLog::log.Size() > error_index) {
			error_display_timer.Restart();
			current_error = "";
			// run a loop to fetch the next 10 errors & print onscreen
			for (int i = 0; i < errors_onscreen_at_once; i++) {
				if (ErrorLog::log.Size() <= error_index)
					break;
				current_error += "Log: " + ErrorLog::log[error_index]->message + '\n';
				error_index++;
			}
		}
		else {
			current_error = "";
		}
	}
	// SETUP TEXT SPRITES
	gfx.spriteBatch->Begin();
	// draw FPS counter
	gfx.spriteFont->DrawString(gfx.spriteBatch.get(),
		StringHelper::StringToWide(fpsString).c_str(), // text 
		DirectX::XMFLOAT2(0, 0), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(1.0f, 1.0f)); // scale
	// draw camera position
	gfx.spriteFont->DrawString(gfx.spriteBatch.get(),
		StringHelper::StringToWide(std::to_string(camera.GetPositionFloat3().x) + ", " + std::to_string(camera.GetPositionFloat3().y) + ", " + std::to_string(camera.GetPositionFloat3().z)).c_str(), // text 
		DirectX::XMFLOAT2(0, 30), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	// draw camera rotation // TODO: get some better numbers lol
	gfx.spriteFont->DrawString(gfx.spriteBatch.get(),
		StringHelper::StringToWide(std::to_string(camera.GetRotationFloat3().x) + ", " + std::to_string(camera.GetRotationFloat3().y) + ", " + std::to_string(camera.GetRotationFloat3().z)).c_str(), // text 
		DirectX::XMFLOAT2(0, 45), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	// draw camera speed
	gfx.spriteFont->DrawString(gfx.spriteBatch.get(),
		StringHelper::StringToWide("speed:" + std::to_string(camera.speed)).c_str(), // text 
		DirectX::XMFLOAT2(0, 60), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	// draw error log
	gfx.spriteFont->DrawString(gfx.spriteBatch.get(),
		StringHelper::StringToWide(current_error).c_str(), // text 
		DirectX::XMFLOAT2(0, 90), // position 
		DirectX::Colors::Red, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	gfx.spriteBatch->End();


	// IMGUI SETUP
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// IMGUI UI CONFIGURATION
	ImGui::Begin("Light Controls");
	ImGui::DragFloat3("Ambient Light Color", &gfx.cb_ps_light.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &gfx.cb_ps_light.data.ambientLightStrenght, 0.0f, 0.0f, 1.0f);
	ImGui::NewLine();
	ImGui::DragFloat3("Directional Light Color", &gfx.cb_ps_light.data.directionalLightColor.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Directional Light Strength", &gfx.cb_ps_light.data.directionalLightStrength, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat3("Directional Light Direction", &gfx.cb_ps_light.data.directionalLightDirection.x, 0.01f, 0.0f, 1.0f);
	ImGui::NewLine();
	//ImGui::DragFloat3("Dynamic Light Color", &gfx.light.lightColor.x, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Strength", &gfx.light.lightStrength, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation A", &gfx.light.attenuation_a, 0.01f, 0.1f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation B", &gfx.light.attenuation_b, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation C", &gfx.light.attenuation_c, 0.01f, 0.0f, 10.0f);
	ImGui::End();



	ui.render_UI(); // call to handle our Slipspace interface UI

	// IM GUI DRAW
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	gfx.swapchain->Present(1, NULL); // 0 for no VSYNC, 1 for VSYNC
	ui.pre_render(camera.GetViewMatrix() * camera.GetProjectionMatrix()); // call to generate/update our rendered images used in the UI


	// just do this after so it doesn't mess up anything
	// it'll have to have a 1frame delay on any input anyway i guess


}