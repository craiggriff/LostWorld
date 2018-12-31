#include "pch.h"
#include "RenderCube.h"

using namespace Game;
using namespace DirectX;

using namespace std;

RenderCube::RenderCube()
{


}
RenderCube::~RenderCube()
{

}



bool RenderCube::Initialize(std::shared_ptr<DX::DeviceResources> pm_deviceResources, int size_width, int size_height, DXGI_FORMAT tex_format)
{
	tex_width = size_width;
	tex_height = size_height;

	m_deviceResources = pm_deviceResources;


	// Create the texture sampler.
	D3D11_SAMPLER_DESC samplerDescription;
	samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.MipLODBias = 0.0f;
	samplerDescription.MaxAnisotropy = 2;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.BorderColor[0] = 0.0f;
	samplerDescription.BorderColor[1] = 0.0f;
	samplerDescription.BorderColor[2] = 0.0f;
	samplerDescription.BorderColor[3] = 0.0f;
	samplerDescription.MinLOD = 0;      // This allows the use of all mip levels.
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateSamplerState(
			&samplerDescription,
			&m_sampler
		)
	); // dont think we need to use this



	// Create environment map.
	D3D11_TEXTURE2D_DESC pointshadowMapTextureDescription;
	ZeroMemory(&pointshadowMapTextureDescription, sizeof(D3D11_TEXTURE2D_DESC));
	pointshadowMapTextureDescription.ArraySize = 6;
	pointshadowMapTextureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	pointshadowMapTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	pointshadowMapTextureDescription.Format = tex_format;
	pointshadowMapTextureDescription.Width = tex_width;
	pointshadowMapTextureDescription.Height = tex_height;
	pointshadowMapTextureDescription.MipLevels = 0;
	pointshadowMapTextureDescription.SampleDesc.Count = 1;
	pointshadowMapTextureDescription.SampleDesc.Quality = 0;
	pointshadowMapTextureDescription.CPUAccessFlags = 0;
	pointshadowMapTextureDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(
			&pointshadowMapTextureDescription,
			0,
			&m_pointshadowMapTexture
		)
	);




	D3D11_RENDER_TARGET_VIEW_DESC pointshadowMapRenderTargetDescription;
	ZeroMemory(&pointshadowMapRenderTargetDescription, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	pointshadowMapRenderTargetDescription.Format = pointshadowMapTextureDescription.Format;
	pointshadowMapRenderTargetDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	pointshadowMapRenderTargetDescription.Texture2DArray.MipSlice = 0;
	pointshadowMapRenderTargetDescription.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < 6; ++i)
	{
		pointshadowMapRenderTargetDescription.Texture2DArray.FirstArraySlice = i;
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateRenderTargetView(
				m_pointshadowMapTexture.Get(),
				&pointshadowMapRenderTargetDescription,
				&m_pointshadowMapRenderTargetView[i]
			)
		);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC pointshadowMapShaderResourceViewDescription;
	ZeroMemory(&pointshadowMapShaderResourceViewDescription, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	pointshadowMapShaderResourceViewDescription.Format = pointshadowMapTextureDescription.Format;
	pointshadowMapShaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	pointshadowMapShaderResourceViewDescription.TextureCube.MostDetailedMip = 0;
	pointshadowMapShaderResourceViewDescription.TextureCube.MipLevels = 1;

	//pointshadowMapShaderResourceViewDescription.nu


	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView(
			m_pointshadowMapTexture.Get(),
			&pointshadowMapShaderResourceViewDescription,
			&m_pointshadowMapShaderResourceView
		)
	);

	CD3D11_TEXTURE2D_DESC environmentMapTextureDepthStencilDescription(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		tex_width,
		tex_height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(
			&environmentMapTextureDepthStencilDescription,
			nullptr,
			&m_pointshadowMapDepthStencilTexture
		)
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC environmentMapDepthStencilViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateDepthStencilView(
			m_pointshadowMapDepthStencilTexture.Get(),
			&environmentMapDepthStencilViewDescription,
			&m_pointshadowMapDepthStencilView
		)
	);

	return true;

}


DirectX::XMMATRIX RenderCube::RenderCubeSide(int _side, DirectX::XMFLOAT4 target_col)
{

	// Create the EnvironmentMap Viewport.
	D3D11_VIEWPORT environmentMapViewport;
	environmentMapViewport.TopLeftX = 0.0f;
	environmentMapViewport.TopLeftY = 0.0f;
	environmentMapViewport.Width = static_cast<float>(tex_width);
	environmentMapViewport.Height = static_cast<float>(tex_height);
	environmentMapViewport.MinDepth = 0.01f;
	environmentMapViewport.MaxDepth = 1.0f;

	// Define the look vector for each Environment Map face.
	DirectX::XMFLOAT3 environmentMapTargets[6] = {
		DirectX::XMFLOAT3(-1, 0, 0),  // Point camera at negative x axis.
		DirectX::XMFLOAT3(1, 0, 0),   // Point camera at positive x axis.
		DirectX::XMFLOAT3(0, 1, 0),   // Point camera at positive y axis.
		DirectX::XMFLOAT3(0, -1, 0),  // Point camera at negative y axis.
		DirectX::XMFLOAT3(0, 0, 1),   // Point camera at positive z axis.
		DirectX::XMFLOAT3(0, 0, -1)   // Point camera at negative z axis.
	};


	// Define the up vector for each Environment Map face.
	DirectX::XMFLOAT3 environmentMapUp[6] = {
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 0, -1),
		DirectX::XMFLOAT3(0, 0, 1),
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 1, 0)
	};
	//return;

	// Start to render the environment map faces, one face at a time.
	m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &environmentMapViewport);
	int view = _side;

	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = target_col.x;
	color[1] = target_col.y;
	color[2] = target_col.z;
	color[3] = target_col.w;

	if (color[3] > 0.0f)
	{
		m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(
			m_pointshadowMapRenderTargetView[view].Get(), color
		);
	}
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(
		m_pointshadowMapDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH , 1.0f, 0);


	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(
		1,
		m_pointshadowMapRenderTargetView[view].GetAddressOf(),
		m_pointshadowMapDepthStencilView.Get()
	);
	
	/*
	m_deviceResources->GetD3DDeviceContext()->PSSetSamplers(
		0,                     // Starting at the first sampler slot.
		1,                     // Set one sampler binding.
		m_sampler.GetAddressOf()
	);
	*/

	return XMMatrixLookAtRH(
		XMLoadFloat3(&XMFLOAT3(render_position.x, render_position.y, render_position.z)),
		XMLoadFloat3(&XMFLOAT3(environmentMapTargets[view].x + render_position.x, environmentMapTargets[view].y + render_position.y, environmentMapTargets[view].z + render_position.z)),
		XMLoadFloat3(&XMFLOAT3(environmentMapUp[view].x, environmentMapUp[view].y, environmentMapUp[view].z))
	);

}


void RenderCube::ClearTargets()
{
	int i;
	for (i = 0; i < 6; i++)
	{


	}
}


void RenderCube::CreateSpotProjection(
	_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
	_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
	_In_ float nearPlane,           // depth to map to 0
	_In_ float farPlane             // depth to map to 1
)
{
	float minScale = 1.0f / tan(minimumFieldOfView * PI_F / 360.0f);
	float xScale = 1.0f;
	float yScale = 1.0f;
	if (aspectRatio < 1.0f)
	{
		xScale = minScale;
		yScale = minScale * aspectRatio;
	}
	else
	{
		xScale = minScale / aspectRatio;
		yScale = minScale;
	}
	float zScale = farPlane / (farPlane - nearPlane);
	spot_projection = DirectX::XMFLOAT4X4(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, -zScale, -nearPlane*zScale,
		0.0f, 0.0f, -1.0f, 0.0f
	);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderCube::GetShaderResourceView()
{
	return m_pointshadowMapShaderResourceView;
}




void RenderCube::Shutdown()
{

}

