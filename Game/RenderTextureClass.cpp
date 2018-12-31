////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "rendertextureclass.h"

using namespace Game;
using namespace DirectX;

using namespace std;

RenderTextureClass::RenderTextureClass()
{
	m_renderTargetTexture = 0;
	m_renderTargetView;// = 0;
	m_shaderResourceView;// = 0;
}


RenderTextureClass::RenderTextureClass(const RenderTextureClass& other)
{
}


RenderTextureClass::~RenderTextureClass()
{
}


bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, DXGI_FORMAT tex_format, int _mips)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	tex_width = textureWidth;
	tex_height = textureHeight;


	CD3D11_TEXTURE2D_DESC environmentMapTextureDepthStencilDescription(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		tex_width,
		tex_height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);


	device->CreateTexture2D(
		&environmentMapTextureDepthStencilDescription,
		nullptr,
		&m_depthStencilTexture
	);



	CD3D11_DEPTH_STENCIL_VIEW_DESC environmentMapDepthStencilViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D);

	device->CreateDepthStencilView(
		m_depthStencilTexture.Get(),
		&environmentMapDepthStencilViewDescription,
		&m_depthStencilView
	);


	/**/
	// Create the EnvironmentMap Viewport.

	environmentMapViewport.TopLeftX = 0.0f;
	environmentMapViewport.TopLeftY = 0.0f;
	environmentMapViewport.Width = static_cast<float>(tex_width);
	environmentMapViewport.Height = static_cast<float>(tex_height);
	environmentMapViewport.MinDepth = 0.0f;
	environmentMapViewport.MaxDepth = 1.0f;



	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = _mips;
	textureDesc.ArraySize = 1;
	textureDesc.Format = tex_format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	if (_mips > 1)
	{
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else
	{
		textureDesc.MiscFlags = 0;
	}
	

	
	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = _mips;
	

	// Create the shader resource view.
	result = device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void RenderTextureClass::Shutdown()
{
#ifndef _DEBUG
	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView;// = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView;;
	}

	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = 0;
	}
#endif
	return;
}

void RenderTextureClass::RestoreRenderTarget(ID3D11DeviceContext* deviceContext)
{

	// Restore old view port.
	deviceContext->RSSetViewports(1, &previousViewport);

	// Restore old render target and depth stencil buffer views.
	deviceContext->OMSetRenderTargets(
		1,
		previousRenderTargetView.GetAddressOf(),
		previousDepthStencilView.Get()
	);
	previousRenderTargetView.ReleaseAndGetAddressOf();
	
}

void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
	deviceContext->OMGetRenderTargets(
		1,
		previousRenderTargetView.GetAddressOf(),
		&previousDepthStencilView
	);

	// Save the old viewport

	UINT numberOfPreviousViewports = 1;
	deviceContext->RSGetViewports(
		&numberOfPreviousViewports,
		&previousViewport
	);


	// Start to render the environment map faces, one face at a time.
	deviceContext->RSSetViewports(1, &environmentMapViewport);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	return;
}


void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* stencil)
{
	deviceContext->OMGetRenderTargets(
		1,
		previousRenderTargetView.GetAddressOf(),
		&previousDepthStencilView
	);

	// Save the old viewport

	UINT numberOfPreviousViewports = 1;
	deviceContext->RSGetViewports(
		&numberOfPreviousViewports,
		&previousViewport
	);
	

	//Start to render the environment map faces, one face at a time.
	deviceContext->RSSetViewports(1, &environmentMapViewport);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), stencil);

	return;
}

void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext* deviceContext,
	float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH , 1.0f, 0);



	return;
}


Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderTextureClass::GetShaderResourceView()
{
	return m_shaderResourceView;
}