#include "Graphics.h"
//#include <WICTextureLoader.h>


bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	try
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if (adapters.size() < 1)
		{
			ErrorLogger::Log("No IDXGI Adapters found.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };

		scd.BufferDesc.Width = this->windowWidth;
		scd.BufferDesc.Height = this->windowHeight;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hwnd;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


		D3D11_CREATE_DEVICE_FLAG device_flags = (D3D11_CREATE_DEVICE_FLAG)0;
		#if defined(DEBUG) || defined(_DEBUG)  
			device_flags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
		#endif

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL, // SOFTWARE DRIVER TYPE
			device_flags, // FLAGS FOR RUNTIME LAYERS
			NULL, // FEATURE LEVELS ARRAY
			0,    // # OF FEATURE LEVELS IN ARRAY
			D3D11_SDK_VERSION,
			&scd, // swap chain descriptionm
			this->swapchain.GetAddressOf(),
			this->device.GetAddressOf(),
			NULL,
			this->deviceContext.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed");

		hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view");

		// STENCIL BUFFER
		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,this->windowWidth,this->windowHeight);
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = this->device->CreateTexture2D(&depthStencilTextureDesc, NULL, this->depthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer");

		hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view");

		this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

		// DEPTH STENCIL

		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state");

		// VIEWPORT SETUP
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(this->windowWidth), static_cast<float>(windowHeight));
		this->deviceContext->RSSetViewports(1, &viewport);

		// RASTERIZER SETUP
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		rasterizerDesc.FrontCounterClockwise = false;
		hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state");

		// RASTERIZER BACK SETUP
		CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
		rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		hr = this->device->CreateRasterizerState(&rasterizerDesc_CullFront, this->rasterizerState_CullFront.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state");

		// BLEND STATE SETUP
		D3D11_BLEND_DESC blendDesc = { 0 };
		D3D11_RENDER_TARGET_BLEND_DESC rtbd{ 0 };

		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.RenderTarget[0] = rtbd;

		hr = this->device->CreateBlendState(&blendDesc, this->blendState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state");

		// SPRITE SETUP
		spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
		spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

		// SAMPLER/TETURE SETUP
		CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state");
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true; // SUCCESSFULL
}

bool Graphics::InitializeShaders()
{
	std::wstring shaderfolder = L"D:\\Projects\\VS\\TagRender\\x64\\Debug\\";
	
	// /////////////////// //
	// CUSTOM HALO SHADER //
	// ///////////////// //
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 1, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 2, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TANGENT", 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },

	};
	if (!vertexshader.Initialize(this->device, shaderfolder + L"vertexshader.cso", layout, ARRAYSIZE(layout)))
		return false;
	if (!pixelshader.Initialize(this->device, shaderfolder + L"pixelshader.cso"))
		return false;

	// //////////////////// //
	// GIZMO OBJECT SHADER //
	// ////////////////// //
	D3D11_INPUT_ELEMENT_DESC generic_layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};
	if (!generic_vertexshader.Initialize(this->device, shaderfolder + L"generic_vertexshader.cso", generic_layout, ARRAYSIZE(generic_layout)))
		return false;
	if (!generic_pixelshader.Initialize(this->device, shaderfolder + L"generic_pixelshader.cso"))
		return false;


	return true;
}

bool Graphics::InitializeScene(){
	try{
		// SETUP SLIPSACE MODULE MANAGER
		// Modules = *(new ModuleManager()); // it is already setup
		/*
		// LOAD TEXTURE
		HRESULT hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\pebbles.png", nullptr, grassTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create wic texture from file");

		// LOAD 2nd TEXTURE
		hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\pink.png", nullptr, pinkTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create wic texture from file");
		*/
		//HRESULT hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\pink.png", nullptr, pinkTexture.GetAddressOf());

		// SETUP CONSTANT BUFFERS
		HRESULT hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get()); //device->CreateBuffer(&desc, 0, constantBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initiate vertexshader");

		hr = this->cb_vs_generic_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get()); //device->CreateBuffer(&desc, 0, constantBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initiate generic vertexshader");

		hr = this->cb_ps_light.Initialize(this->device.Get(), this->deviceContext.Get()); //device->CreateBuffer(&desc, 0, constantBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initiate pixelshader");

		this->cb_ps_light.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		this->cb_ps_light.data.ambientLightStrenght = 1.0f;

		this->cb_ps_light.data.directionalLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		this->cb_ps_light.data.directionalLightStrength = 1.0f;
		this->cb_ps_light.data.directionalLightDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);


		
		// SETUP MODELS
		//if (!gameObject.Initialize("Data\\Objects\\Samples\\dodge_challenger.fbx", this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader))
		//	return false;

		//if (!light.Initialize(this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader))
	    //	return false;
		

		camera.SetPosition(0.0f, 0.0f, -2.0f);
		camera.SetProjectionValues(
			90.0f,
			static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
			0.1f,
			300000.0f);
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Graphics::RenderFrame()
{
	this->cb_vs_vertexshader.data.camera_position = camera.GetPositionFloat3();
	this->cb_vs_generic_vertexshader.data.camera_position = camera.GetPositionFloat3();


	// SETUP LIGHTING
	//this->cb_ps_light.data.dynamicLightColor = light.lightColor;
	//this->cb_ps_light.data.dynamicLightStrength = light.lightStrength;
	//this->cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	//this->cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	//this->cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	//this->cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;

	//this->cb_ps_light.ApplyChanges();
	//this->deviceContext->PSSetConstantBuffers(0, 1, this->cb_ps_light.GetAddressOf());



	// setup background stuff
	float bgcolor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	this->deviceContext->IASetInputLayout(this->generic_vertexshader.GetInputLayout());
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceContext->RSSetState(this->rasterizerState.Get());
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	//this->deviceContext->OMSetBlendState(this->blendState.Get(), NULL, 0xFFFFFFFF);
	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
	this->deviceContext->VSSetShader(generic_vertexshader.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(generic_pixelshader.GetShader(), NULL, 0);

	// render a test element
	Vertex v[] =
	{
		Vertex(-0.5f,-0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,0.0f), // FRONT bottom left
		Vertex(-0.5f, 0.5f,-0.5f,  0.0f,0.0f,  0.0f,0.0f,0.0f), // FRONT top left
		Vertex( 0.5f, 0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,0.0f), // FRONT top right
		Vertex( 0.5f,-0.5f,-0.5f,  1.0f,1.0f,  0.0f,0.0f,0.0f), // FRONT bottom right

		Vertex(-0.5f,-0.5f, 0.5f,  0.0f,1.0f,  0.0f,0.0f,0.0f), // BACK bottom left
		Vertex(-0.5f, 0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,0.0f), // BACK top left
		Vertex( 0.5f, 0.5f, 0.5f,  1.0f,0.0f,  0.0f,0.0f,0.0f), // BACK top right
		Vertex( 0.5f,-0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,0.0f), // BACK bottom right
	};

	VertexBuffer<Vertex> test_vert_buffer = {};
	// LOAD VERTEX DATA
	HRESULT hr = test_vert_buffer.Initialize(device.Get(), v, ARRAYSIZE(v));   // this->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, this->vertexBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create vertex buffer");

	DWORD indicies[] =
	{
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
	hr = test_index_buffer.Initialize(device.Get(), indicies, ARRAYSIZE(indicies)); // device->CreateBuffer(&indexBufferDesc, &indexBufferData, indiciesBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create indicies buffer");

	// render test data
	this->deviceContext->VSSetConstantBuffers(0, 1, cb_vs_generic_vertexshader.GetAddressOf());


	XMMATRIX worldMatrix = XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(0, 0, 0);

	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_generic_vertexshader.GetAddressOf());
	cb_vs_generic_vertexshader.data.wvpMatrix = DirectX::XMMatrixIdentity() * worldMatrix * (camera.GetViewMatrix() * camera.GetProjectionMatrix());
	cb_vs_generic_vertexshader.data.worldMatrix = DirectX::XMMatrixIdentity() * worldMatrix;
	cb_vs_generic_vertexshader.ApplyChanges();



	UINT offset = 0;
	this->deviceContext->IASetVertexBuffers(0, 1, test_vert_buffer.GetAddressOf(), test_vert_buffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(test_index_buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(test_index_buffer.IndexCount(), 0, 0);
	// end test section

	this->deviceContext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);


	//static float alpha = 0.5f;
	{ // 

		//this->gameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());

		// PIXEL SHADER
		//this->cb_vs_pixelshader.data.alpha = 1.0f;
		//this->cb_vs_pixelshader.ApplyChanges();
		//this->deviceContext->PSSetConstantBuffers(0, 1, this->cb_vs_pixelshader.GetAddressOf());

	}
	{

		//this->deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		//this->light.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());

	}

	// testing junk
	ui.render_GEO(&Modules, device.Get(), deviceContext.Get(), &cb_vs_vertexshader, camera.GetViewMatrix() * camera.GetProjectionMatrix()); // call to handle our Slipspace interface UI





	// SETUP FPS COUNTER
	static int fpsCoutner = 0;
	static std::string fpsString = "FPS: null";
	fpsCoutner += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0)
	{
		fpsString = "FPS: " + std::to_string(fpsCoutner);
		fpsCoutner = 0;
		fpsTimer.Restart();
	}
	// SETUP TEXT SPRITES
	spriteBatch->Begin();
	// draw FPS counter
	spriteFont->DrawString(spriteBatch.get(),
		StringHelper::StringToWide(fpsString).c_str(), // text 
		DirectX::XMFLOAT2(0, 0), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(1.0f, 1.0f)); // scale
	// draw camera position
	spriteFont->DrawString(spriteBatch.get(),
		StringHelper::StringToWide(std::to_string(camera.GetPositionFloat3().x) + ", " + std::to_string(camera.GetPositionFloat3().y) + ", " + std::to_string(camera.GetPositionFloat3().z)).c_str(), // text 
		DirectX::XMFLOAT2(0, 30), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	// draw camera rotation // TODO: get some better numbers lol
	spriteFont->DrawString(spriteBatch.get(),
		StringHelper::StringToWide(std::to_string(camera.GetRotationFloat3().x) + ", " + std::to_string(camera.GetRotationFloat3().y) + ", " + std::to_string(camera.GetRotationFloat3().z)).c_str(), // text 
		DirectX::XMFLOAT2(0, 45), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	// draw camera speed
	spriteFont->DrawString(spriteBatch.get(),
		StringHelper::StringToWide("speed:" + std::to_string(camera.speed)).c_str(), // text 
		DirectX::XMFLOAT2(0, 60), // position 
		DirectX::Colors::White, // color
		0.0f, // rotation
		DirectX::XMFLOAT2(0.0f, 0.0f), // origin
		DirectX::XMFLOAT2(0.5f, 0.5f)); // scale
	spriteBatch->End();


	// IMGUI SETUP
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// IMGUI UI CONFIGURATION
	ImGui::Begin("Light Controls");
	ImGui::DragFloat3("Ambient Light Color", &this->cb_ps_light.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &this->cb_ps_light.data.ambientLightStrenght, 0.0f, 0.0f, 1.0f);
	ImGui::NewLine();
	ImGui::DragFloat3("Directional Light Color", &this->cb_ps_light.data.directionalLightColor.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Directional Light Strength", &this->cb_ps_light.data.directionalLightStrength, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat3("Directional Light Direction", &this->cb_ps_light.data.directionalLightDirection.x, 0.01f, 0.0f, 1.0f);
	ImGui::NewLine();
	//ImGui::DragFloat3("Dynamic Light Color", &this->light.lightColor.x, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Strength", &this->light.lightStrength, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation A", &this->light.attenuation_a, 0.01f, 0.1f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation B", &this->light.attenuation_b, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat("Dynamic Light Attenuation C", &this->light.attenuation_c, 0.01f, 0.0f, 10.0f);
	ImGui::End();



	ui.render_UI(&Modules, device.Get(), deviceContext.Get()); // call to handle our Slipspace interface UI
	
	// IM GUI DRAW
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	this->swapchain->Present(1, NULL); // 0 for no VSYNC, 1 for VSYNC
}

void pushnextConsole() {

}
void PrintConsoleQueue() {

}
