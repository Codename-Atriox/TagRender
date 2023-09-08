#pragma once

#include "AdapterReader.h"
#include "Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
//#include <WICTextureLoader.h>
#include "..\\Timer.h"
#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"
//#include "GameObject.h"
//#include "RenderableGameObject.h"
//#include "Light.h"
#include "Camera.h" // too lazy to fix this. // holy i hate c++ includes
#include "../Slipspace/Tags/TagProcessor.h"


class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
// these are public so we dont have to pass 1 million parameters down the UI pipeline
public:
	Microsoft::WRL::ComPtr <ID3D11Device> device;
	Microsoft::WRL::ComPtr <ID3D11DeviceContext> deviceContext;

	Microsoft::WRL::ComPtr <ID3D11InputLayout> IntputLayout;

	ConstantBuffer<CB_VS_generic_vertexshader> cb_vs_generic_vertexshader;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;

	ModuleManager Modules;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	Microsoft::WRL::ComPtr <IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> renderTargetView;

	VertexShader generic_vertexshader;
	VertexShader vertexshader;
	PixelShader pixelshader;
	PixelShader generic_pixelshader;
	ConstantBuffer<CB_PS_light> cb_ps_light;
	//Microsoft::WRL::ComPtr <ID3D10Blob> vertex_shader_buffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;



	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;


	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pinkTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grassTexture;

	int windowWidth = 0;
	int windowHeight = 0;

};
