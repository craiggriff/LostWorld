////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "rendertexturearray.h"

using namespace Game;
using namespace DirectX;

using namespace std;

RenderTextureArray::RenderTextureArray()
{
	//m_renderTargetTexture = 0;
	m_renderTargetView;// = 0;
	m_shaderResourceView;// = 0;
}


RenderTextureArray::RenderTextureArray(const RenderTextureArray& other)
{
}


RenderTextureArray::~RenderTextureArray()
{
}


bool RenderTextureArray::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, int num_textures, DXGI_FORMAT tex_format)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	tex_width = textureWidth;
	tex_height = textureHeight;
	tex_num = num_textures;


	CD3D11_TEXTURE2D_DESC environmentMapTextureDepthStencilDescription(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		tex_width,
		tex_height,
		tex_num,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);


	device->CreateTexture2D(
		&environmentMapTextureDepthStencilDescription,
		nullptr,
		&m_environmentMapDepthStencilTexture
	);



	CD3D11_DEPTH_STENCIL_VIEW_DESC environmentMapDepthStencilViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D);

	device->CreateDepthStencilView(
		m_environmentMapDepthStencilTexture.Get(),
		&environmentMapDepthStencilViewDescription,
		&m_environmentMapDepthStencilView
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
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = tex_num;
	textureDesc.Format = tex_format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;


	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	/*
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
	*/

	m_renderTargetView = new Microsoft::WRL::ComPtr<ID3D11RenderTargetView>[tex_num];

	D3D11_RENDER_TARGET_VIEW_DESC pointshadowMapRenderTargetDescription;
	ZeroMemory(&pointshadowMapRenderTargetDescription, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	pointshadowMapRenderTargetDescription.Format = tex_format;
	pointshadowMapRenderTargetDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	pointshadowMapRenderTargetDescription.Texture2DArray.MipSlice = 0;
	pointshadowMapRenderTargetDescription.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < tex_num; ++i)
	{
		pointshadowMapRenderTargetDescription.Texture2DArray.FirstArraySlice = i;
		DX::ThrowIfFailed(
			device->CreateRenderTargetView(
				m_renderTargetTexture.Get(),
				&pointshadowMapRenderTargetDescription,
				&m_renderTargetView[i]
			)
		);
	}



	D3D11_SHADER_RESOURCE_VIEW_DESC pointshadowMapShaderResourceViewDescription;
	ZeroMemory(&pointshadowMapShaderResourceViewDescription, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	pointshadowMapShaderResourceViewDescription.Format = tex_format;
	pointshadowMapShaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	pointshadowMapShaderResourceViewDescription.Texture2DArray.MostDetailedMip = 0;

	pointshadowMapShaderResourceViewDescription.Texture2DArray.MipLevels = 1;


	//pointshadowMapShaderResourceViewDescription.nu

	for (int i = 0; i < tex_num; ++i)
	{
		pointshadowMapShaderResourceViewDescription.Texture2DArray.FirstArraySlice = i;
		DX::ThrowIfFailed(
			device->CreateShaderResourceView(
				m_renderTargetTexture.Get(),
				&pointshadowMapShaderResourceViewDescription,
				&m_shaderResourceView[i]
			)
		);
	}
	/*
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
	{
		return false;
	}
	*/

	return true;
}


void RenderTextureArray::Shutdown()
{
#ifndef _DEBUG
	if (m_shaderResourceView)
	{
		//m_shaderResourceView->Release();
		m_shaderResourceView;// = 0;
	}

	if (m_renderTargetView)
	{
		//m_renderTargetView->Release();
		m_renderTargetView;;
	}

	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		//m_renderTargetTexture = 0;
	}
#endif
	return;
}


void RenderTextureArray::SetRenderTarget(int _num, ID3D11DeviceContext* deviceContext)
{


	// Start to render the environment map faces, one face at a time.
	deviceContext->RSSetViewports(1, &environmentMapViewport);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, m_renderTargetView[_num].GetAddressOf(), m_environmentMapDepthStencilView.Get());

	return;
}


void RenderTextureArray::SetRenderTarget(int _num,ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* stencil)
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
	deviceContext->OMSetRenderTargets(1, m_renderTargetView[_num].GetAddressOf(), stencil);

	return;
}

void RenderTextureArray::ClearRenderTarget(int _num, ID3D11DeviceContext* deviceContext,
	float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(m_renderTargetView[_num].Get(), color);

	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(m_environmentMapDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);



	return;
}


ID3D11ShaderResourceView* RenderTextureArray::GetShaderResourceView()
{
	return m_shaderResourceView[0];
}