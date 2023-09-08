#pragma once
#include "../../Logic/RenderGeometry.h"

class ModelWindow {
private:
	Graphics* gfx; // so we can access all those annoying things

	ID3D11Texture2D* renderTargetTextureMap;
	ID3D11RenderTargetView* renderTargetViewMap;

	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilView* depthStencilView;

	XMMATRIX mapView;
	XMMATRIX mapProjection;

	const float background[4] = { 0.9f, 0.3f, 0.3f, 1.0f };
public:
	ID3D11ShaderResourceView* shaderResourceViewMap; // so we can reference it with ImGui

	ModelWindow(Graphics* gfx, Tag* obj, uint32_t width, uint32_t height) {
		this->gfx = gfx;
		this->obj = obj;
		InitTexture(width, height);
	}

	XMFLOAT3 pos;
	XMFLOAT3 rot;

	int mesh_index;
	int lod_index;

	Tag* obj;

	void destroy() {
		if (renderTargetTextureMap) {
			renderTargetTextureMap->Release();
			delete renderTargetTextureMap;}
		if (renderTargetViewMap) {
			renderTargetViewMap->Release();
			delete renderTargetViewMap;}
		if (shaderResourceViewMap) {
			shaderResourceViewMap->Release();
			delete shaderResourceViewMap;}
	}
	void UpdateTexture() {
		gfx->deviceContext->OMSetRenderTargets(1, &renderTargetViewMap, depthStencilView);
		// convert position & rotation into matrix
		mapView = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);

		gfx->deviceContext->OMSetDepthStencilState(depthStencilState, 0);
		// set render target & clear it
		gfx->deviceContext->ClearRenderTargetView(renderTargetViewMap, background);
		gfx->deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// DEBUG // DEBUG // DEBUG //
			// render a test element
		Vertex v[] = {
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
		HRESULT hr = test_vert_buffer.Initialize(gfx->device.Get(), v, ARRAYSIZE(v));   // gfx->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, gfx->vertexBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create vertex buffer");

		DWORD indicies[] = {
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
		hr = test_index_buffer.Initialize(gfx->device.Get(), indicies, ARRAYSIZE(indicies)); // device->CreateBuffer(&indexBufferDesc, &indexBufferData, indiciesBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create indicies buffer");

		// render test data
		gfx->deviceContext->VSSetConstantBuffers(0, 1, gfx->cb_vs_generic_vertexshader.GetAddressOf());


		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(0, 0, 0);

		gfx->deviceContext->VSSetConstantBuffers(0, 1, gfx->cb_vs_generic_vertexshader.GetAddressOf());
		gfx->cb_vs_generic_vertexshader.data.wvpMatrix = DirectX::XMMatrixIdentity() * worldMatrix * (mapView * mapProjection);
		gfx->cb_vs_generic_vertexshader.data.worldMatrix = DirectX::XMMatrixIdentity() * worldMatrix;
		gfx->cb_vs_generic_vertexshader.ApplyChanges();



		UINT offset = 0;
		gfx->deviceContext->IASetVertexBuffers(0, 1, test_vert_buffer.GetAddressOf(), test_vert_buffer.StridePtr(), &offset);
		gfx->deviceContext->IASetIndexBuffer(test_index_buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		gfx->deviceContext->DrawIndexed(test_index_buffer.IndexCount(), 0, 0);
		// end test section

		gfx->deviceContext->IASetInputLayout(gfx->vertexshader.GetInputLayout());
		gfx->deviceContext->VSSetShader(gfx->vertexshader.GetShader(), NULL, 0);
		gfx->deviceContext->PSSetShader(gfx->pixelshader.GetShader(), NULL, 0);

		// DEBUG // DEBUG // DEBUG //











		RenderGeometry::render(obj, gfx, mapView, mapProjection, mesh_index, lod_index);
	}
	void InitTexture(uint32_t width, uint32_t height) {
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ZeroMemory(&textureDesc, sizeof(textureDesc));
		ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));

		// Setup the texture description.
		// We will have our map be a square
		// We will need to have this texture bound as a render target AND a shader resource
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the texture
		HRESULT hr = gfx->device->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureMap);
		if (FAILED(hr)) throw exception("failed to generate texture2D for UI model");

		// Setup the description of the render target view.
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		//hr = gfx->device->CreateRenderTargetView(renderTargetTextureMap, nullptr, &renderTargetViewMap);
		hr = gfx->device->CreateRenderTargetView(renderTargetTextureMap, &renderTargetViewDesc, &renderTargetViewMap);
		if (FAILED(hr)) throw exception("failed to generate render target view for UI model");

		/////////////////////// Map's Shader Resource View
		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = -1;


		// Create the shader resource view.
		//hr = gfx->device->CreateShaderResourceView(renderTargetTextureMap, nullptr, &shaderResourceViewMap);
		hr = gfx->device->CreateShaderResourceView(renderTargetTextureMap, &shaderResourceViewDesc, &shaderResourceViewMap);
		if (FAILED(hr)) throw exception("failed to generate render target resource for UI model");




		// STENCIL BUFFER
		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height);
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = gfx->device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilBuffer);
		if (FAILED(hr)) throw exception("Failed to create depth stencil buffer");
		// im not sure if we need this or not
		hr = gfx->device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
		if (FAILED(hr)) throw exception("Failed to create depth stencil view");

		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		hr = gfx->device->CreateDepthStencilState(&depthstencildesc, &depthStencilState);
		if (FAILED(hr)) throw exception("Failed to create depth stencil state");

		// Build an orthographic projection matrix

		

		float fovRadians = (90.0f / 360.0f) * XM_2PI;

		mapProjection = XMMatrixPerspectiveFovLH(fovRadians, 1.0f, 0.1f, 300000.0f);
		//mapProjection = XMMatrixOrthographicLH(512, 512, 1.0f, 1000.0f);

		XMVECTOR mapCamUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		//mapView = XMMatrixLookAtLH(mapCamPosition, mapCamTarget, mapCamUp);

		// complete a single render
		//UpdateTexture();
	}
};

class ModelsManager {
public:
	void AddWindow(Tag* tag, Graphics* gfx) {
		if (!IsOpen(tag))
			// then open it & init window
			Windows.Append(new ModelWindow(gfx, tag, 256, 256));
	}
	void RemoveWindow(Tag* tag) {
		int target = GetWindowIndex(tag);
		if (target == -1)
			throw exception("cannot close window that is not open!!!");
		Windows[target]->destroy();
		Windows.RemoveAt(target);
	}
	// so we can determine whether to show that button thing
	bool IsOpen(Tag* tag) {
		return (GetWindowIndex(tag) != -1);
	}
	// must be called before rending the DX11 scene, as it will interfere with the render targets
	void Prerender() {
		for (int i = 0; i < Windows.Size(); i++) {
			ModelWindow* curr_window = Windows[i];
			curr_window->UpdateTexture();
		}
	}
	void RenderWindows() {
		for (int i = 0; i < Windows.Size(); i++) {
			ModelWindow* curr_window = Windows[i];
			// render the ImGui & the scene
			ImGui::PushID(curr_window->obj->tagID);
			bool window_is_open = true;
			if (!ImGui::Begin(curr_window->obj->tagname.c_str(), &window_is_open) && window_is_open) {
				ImGui::End();
				ImGui::PopID();
				continue;
			}
			ImGui::DragInt("Active Mesh", &curr_window->mesh_index, 0.1f, 0, 100);
			ImGui::DragInt("Active Lod", &curr_window->lod_index, 0.1f, 0, 100);
			ImGui::DragFloat3("Pos", &curr_window->pos.x, 0.3);
			ImGui::DragFloat3("Rot", &curr_window->rot.x, 0.3);

			ImGui::Image((void*)curr_window->shaderResourceViewMap, ImVec2(256, 256));
			//ImGui::DragInt("Active Part", &curr_window->, 0.1f, 0, 100);
			ImGui::End();
			ImGui::PopID();
		}
	}
private:
	int GetWindowIndex(Tag* tag) {
		for (int i = 0; i < Windows.Size(); i++)
			if (Windows[i]->obj == tag)
				return i;
		return -1;
	}
	CTList<ModelWindow> Windows;
	
};
