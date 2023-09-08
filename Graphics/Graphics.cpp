#include "Graphics.h"
//#include <WICTextureLoader.h>


bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;

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
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 1, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 1, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 2, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 2, DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT, 3, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"COLOR",	 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 4, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL",	 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 5, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TANGENT",	 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 6, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },

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
		

	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}



