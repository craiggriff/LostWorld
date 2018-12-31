
#include "pch.h"
#include "DefParticle.h"
#include "RenderCubeArray.h"


using namespace Game;
using namespace DirectX;

using namespace std;

RenderCubeArray::RenderCubeArray()
{


}

RenderCubeArray::~RenderCubeArray()
{

}



bool RenderCubeArray::Initialize(std::shared_ptr<DX::DeviceResources> pm_deviceResources, int size_width, int size_height, int num_cubes, DXGI_FORMAT tex_format, int size_width_to , int size_height_to )
{
	tex_width = size_width;
	tex_height = size_height;
	total_cubes = num_cubes;

	m_deviceResources = pm_deviceResources;

	render_position = new DirectX::XMFLOAT3[total_cubes];


	   // Create environment map.
	D3D11_TEXTURE2D_DESC pointshadowMapTextureDescription;
	ZeroMemory(&pointshadowMapTextureDescription, sizeof(D3D11_TEXTURE2D_DESC));
	pointshadowMapTextureDescription.ArraySize = 6 * total_cubes;
	pointshadowMapTextureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	pointshadowMapTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	pointshadowMapTextureDescription.Format = tex_format;
	pointshadowMapTextureDescription.Width = tex_width;
	pointshadowMapTextureDescription.Height = tex_height;
	pointshadowMapTextureDescription.MipLevels = 0;
	pointshadowMapTextureDescription.SampleDesc.Count = 1;
	pointshadowMapTextureDescription.SampleDesc.Quality = 0;
	pointshadowMapTextureDescription.CPUAccessFlags = 0;
	//pointshadowMapTextureDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
	pointshadowMapTextureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(
			&pointshadowMapTextureDescription,
			0,
			&m_renderTexture
		)
	);

	m_renderTargetView = new Microsoft::WRL::ComPtr<ID3D11RenderTargetView>[6 * total_cubes];




	D3D11_RENDER_TARGET_VIEW_DESC pointshadowMapRenderTargetDescription;
	ZeroMemory(&pointshadowMapRenderTargetDescription, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	pointshadowMapRenderTargetDescription.Format = pointshadowMapTextureDescription.Format;
	pointshadowMapRenderTargetDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	pointshadowMapRenderTargetDescription.Texture2DArray.MipSlice = 0;
	pointshadowMapRenderTargetDescription.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < 6 * total_cubes; ++i)
	{
		pointshadowMapRenderTargetDescription.Texture2DArray.FirstArraySlice = i;
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateRenderTargetView(
				m_renderTexture.Get(),
				&pointshadowMapRenderTargetDescription,
				&m_renderTargetView[i]
			)
		);
	}


	//SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	//SRVDesc.TextureCubeArray.MipLevels = numMips;
	//SRVDesc.TextureCubeArray.MostDetailedMip = 0;
	//SRVDesc.TextureCubeArray.First2DArrayFace = 0;
	//SRVDesc.TextureCubeArray.NumCubes = numLevels;

	D3D11_SHADER_RESOURCE_VIEW_DESC pointshadowMapShaderResourceViewDescription;
	ZeroMemory(&pointshadowMapShaderResourceViewDescription, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	pointshadowMapShaderResourceViewDescription.Format = pointshadowMapTextureDescription.Format;
	pointshadowMapShaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	pointshadowMapShaderResourceViewDescription.TextureCubeArray.MostDetailedMip = 0;
	pointshadowMapShaderResourceViewDescription.TextureCubeArray.First2DArrayFace = 0;
	pointshadowMapShaderResourceViewDescription.TextureCubeArray.MipLevels = 1;
	pointshadowMapShaderResourceViewDescription.TextureCubeArray.NumCubes = 1;

	//pointshadowMapShaderResourceViewDescription.nu

	for (int i = 0; i < total_cubes; i++)
	{
		pointshadowMapShaderResourceViewDescription.TextureCubeArray.First2DArrayFace = i * 6;
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(
				m_renderTexture.Get(),
				&pointshadowMapShaderResourceViewDescription,
				&m_shaderResourceView[i]
			)
		);
	}

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
			&m_depthStencilTexture
		)
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC environmentMapDepthStencilViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateDepthStencilView(
			m_depthStencilTexture.Get(),
			&environmentMapDepthStencilViewDescription,
			&m_depthStencilView
		)
	);

	return true;

}


DirectX::XMMATRIX RenderCubeArray::RenderCubeSide(int _cube_num, int _side)
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


	// Start to render the environment map faces, one face at a time.
	m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &environmentMapViewport);
	//int view = _side;// (6 * _cube_num) + _side;

	float color[4];
	//wd_cube_num = 0;

	// Setup the color to clear the buffer to.
	color[0] = 1.0f;
	color[1] = 0.0f;
	color[2] = 0.0f;
	color[3] = 1.0f;

	//if (_cube_num == 1)exit(0);
	//_cube_num = 1;
	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(
		m_renderTargetView[_side + (6 * _cube_num)].Get(), color
	);

	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH, 1.0f, 0);


	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(
		1,
		m_renderTargetView[_side + (6 * _cube_num)].GetAddressOf(),
		m_depthStencilView.Get()
	);


	return XMMatrixLookAtRH(
		XMLoadFloat3(&XMFLOAT3(render_position[_cube_num].x, render_position[_cube_num].y, render_position[_cube_num].z)),

		XMLoadFloat3(&XMFLOAT3(environmentMapTargets[_side].x + render_position[_cube_num].x, 
								environmentMapTargets[_side].y + render_position[_cube_num].y, 
								environmentMapTargets[_side].z + render_position[_cube_num].z)),

		XMLoadFloat3(&XMFLOAT3(environmentMapUp[_side].x, environmentMapUp[_side].y, environmentMapUp[_side].z))
	);

}


void RenderCubeArray::ClearTargets()
{
	int i;
	for (i = 0; i < 6; i++)
	{


	}
}


void RenderCubeArray::CreateSpotProjection(
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

/*
ID3D11ShaderResourceView* RenderCubeArray::GetShaderResourceView()
{
	return m_pointshadowMapShaderResourceView;
}
*/



void RenderCubeArray::Shutdown()
{

}

