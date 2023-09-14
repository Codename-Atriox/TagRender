#pragma once
#include "../Slipspace/Tags/TagProcessor.h"

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
			renderTargetTextureMap->Release();}
		if (renderTargetViewMap) {
			renderTargetViewMap->Release();}
		if (shaderResourceViewMap) {
			shaderResourceViewMap->Release();}
	}
	void UpdateTexture(XMMATRIX projection) {
		gfx->deviceContext->OMSetRenderTargets(1, &renderTargetViewMap, depthStencilView);
		// convert position & rotation into matrix

		gfx->deviceContext->OMSetDepthStencilState(depthStencilState, 0);
		// set render target & clear it
		gfx->deviceContext->ClearRenderTargetView(renderTargetViewMap, background);
		gfx->deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		//XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0,0,0) * XMMatrixTranslation(0,0,0);
		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x, pos.z, pos.y);
		// restore renderer
		gfx->deviceContext->VSSetConstantBuffers(0, 1, gfx->cb_vs_vertexshader.GetAddressOf());
		gfx->deviceContext->IASetInputLayout(gfx->vertexshader.GetInputLayout());
		gfx->deviceContext->VSSetShader(gfx->vertexshader.GetShader(), NULL, 0);
		gfx->deviceContext->PSSetShader(gfx->pixelshader.GetShader(), NULL, 0);





		//XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

		//// calc camera vector?
		//XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
		//// adjust cam position
		//XMVECTOR posVector = XMLoadFloat3(&pos);
		//camTarget += posVector;
		//// calc up direction
		//XMVECTOR upDir = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), camRotationMatrix);
		//// rebuild the view matrix
		//mapView = XMMatrixLookAtLH(posVector, camTarget, upDir);








		XMMATRIX projec_matrix = mapView* mapProjection;

		RenderGeometry::render(obj, gfx, worldMatrix, projec_matrix, XMFLOAT3(0,0,0), mesh_index, lod_index);
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


		

		float fovRadians = (90.0f / 360.0f) * XM_2PI;

		//mapProjection = XMMatrixOrthographicLH(512, 512, 1.0f, 1000.0f);
		mapProjection = XMMatrixPerspectiveFovLH(fovRadians, 1.0f, 0.1f, 300000.0f);

		//XMVECTOR mapCamUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		//mapView = XMMatrixLookAtLH(mapCamPosition, mapCamTarget, mapCamUp);

		// complete a single render
		//UpdateTexture();
		// calc rotation
		XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(0.0, 0.0, 0.0);

		// calc camera vector?
		XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRotationMatrix);
		// adjust cam position
		XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, -2.0f);
		XMVECTOR posVector = XMLoadFloat3(&pos);
		camTarget += posVector;
		// calc up direction
		XMVECTOR upDir = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), camRotationMatrix);
		// rebuild the view matrix
		mapView = XMMatrixLookAtLH(posVector, camTarget, upDir);



		//const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		//const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}
};

class ModelsManager {
public:
	void AddWindow(Tag* tag, Graphics* gfx) {
		if (!IsOpen(tag))
			// then open it & init window
			Windows.Append(new ModelWindow(gfx, tag, 1024, 1024));
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
	void Prerender(XMMATRIX projection) {
		for (int i = 0; i < Windows.Size(); i++) {
			ModelWindow* curr_window = Windows[i];
			curr_window->UpdateTexture(projection);
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
			if (!window_is_open) {
				// basically just the remove window function but without the getting overhead
				curr_window->destroy();
				Windows.RemoveAt(i);
				i--; // remember to go back an index
				ImGui::End();
				ImGui::PopID();
				continue;
			}
			ImGui::Text("Meshes [%d]", RenderGeometry::get_mesh_count(curr_window->obj));
			ImGui::Text("LODs [%d]", RenderGeometry::get_lod_count(curr_window->obj, curr_window->mesh_index));
			ImGui::Text("Parts [%d]", RenderGeometry::get_parts_count(curr_window->obj, curr_window->mesh_index, curr_window->lod_index));
			ImGui::Text("Verts [%d]", RenderGeometry::get_verts_count(curr_window->obj, curr_window->mesh_index, curr_window->lod_index));

			ImGui::DragInt("Active Mesh", &curr_window->mesh_index, 0.1f, 0, 100);
			ImGui::DragInt("Active Lod", &curr_window->lod_index, 0.1f, 0, 100);
			ImGui::DragFloat3("Pos", &curr_window->pos.x, 0.05f);
			ImGui::DragFloat3("Rot", &curr_window->rot.x, 0.05f);

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
