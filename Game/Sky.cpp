#include "pch.h"
#include "Sky.h"


using namespace Game;
using namespace DirectX;
using namespace std;


Sky::Sky(AllResources* pm_Resources)
{
	m_Resources = pm_Resources;

	m_deviceResources = m_Resources->m_deviceResources;
	m_Texture = nullptr;
	m_loadingComplete = false;

}


void Sky::Initialize(float _size, float _ypos)
{
	pos_y = _ypos;

	m_Matrix = m_Resources->MakeMatrix(0.0f, _ypos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	float squash_y = 1.0f;

	float box_size = _size;

	// Load mesh vertices. Each vertex has a position and a color.
	static const VertexPositionTex cubeVertices[] =
	{
		//1
		{ XMFLOAT3(box_size, box_size *squash_y, -box_size), XMFLOAT2(1.0f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(-box_size, box_size*squash_y,-box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.75f) },
		{ XMFLOAT3(-box_size, box_size*squash_y, box_size), XMFLOAT2(0.6666667f, 1.0f - 0.5f) },
		//2
		{ XMFLOAT3(box_size, box_size*squash_y, -box_size), XMFLOAT2( 1.0f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(-box_size, box_size*squash_y, box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.5f) },
		{ XMFLOAT3(box_size, box_size*squash_y, box_size), XMFLOAT2( 1.0f, 1.0f - 0.5f) },
		//3
		{ XMFLOAT3(-box_size, -box_size*squash_y, box_size), XMFLOAT2( 0.3333333f, 1.0f - 0.5f) }, // - x
		{ XMFLOAT3(box_size, -box_size*squash_y, box_size),  XMFLOAT2( 0.3333333f, 1.0f - 0.25f) },
		{ XMFLOAT3(box_size, box_size*squash_y, box_size),   XMFLOAT2( 0.6666667f, 1.0f - 0.25f) },
		//4
		{ XMFLOAT3(-box_size, -box_size*squash_y, box_size), XMFLOAT2( 0.3333333f, 1.0f - 0.5f) }, // - x
		{ XMFLOAT3(box_size, box_size*squash_y, box_size),  XMFLOAT2( 0.6666667f, 1.0f - 0.25f) },
		{ XMFLOAT3(-box_size, box_size*squash_y, box_size),   XMFLOAT2( 0.6666667f,1.0f - 0.5f) },
		//5
		{ XMFLOAT3(-box_size, -box_size*squash_y, -box_size), XMFLOAT2( 0.3333333f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(box_size, -box_size*squash_y, -box_size),  XMFLOAT2( 0.0f,1.0f - 0.75f) },
		{ XMFLOAT3(-box_size, -box_size*squash_y, box_size),   XMFLOAT2( 0.3333333f, 1.0f - 0.5f) },
		//6
		{ XMFLOAT3(box_size, -box_size*squash_y, -box_size), XMFLOAT2( 0.0f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(box_size, -box_size*squash_y, box_size),  XMFLOAT2( 0.0f, 1.0f - 0.5f) },
		{ XMFLOAT3(-box_size, -box_size*squash_y, box_size),   XMFLOAT2( 0.3333333f, 1.0f - 0.5f) },
		//7
		{ XMFLOAT3(-box_size, box_size*squash_y, -box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(box_size, box_size*squash_y, -box_size),  XMFLOAT2( 0.6666667f, 1.0f - 1.0f) },
		{ XMFLOAT3(-box_size, -box_size*squash_y, -box_size),   XMFLOAT2( 0.333333f, 1.0f - 0.75f) },
		//8
		{ XMFLOAT3(box_size, box_size*squash_y, -box_size), XMFLOAT2( 0.6666667f, 1.0f - 1.0f) }, // - x
		{ XMFLOAT3(box_size, -box_size*squash_y, -box_size),  XMFLOAT2( 0.3333333f, 1.0f - 1.0f) },
		{ XMFLOAT3(-box_size, -box_size*squash_y, -box_size),   XMFLOAT2( 0.3333333f, 1.0f - 0.75f) },
		//9
		{ XMFLOAT3(box_size, box_size*squash_y, -box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.0f) }, // - x
		{ XMFLOAT3(box_size, box_size*squash_y, box_size),  XMFLOAT2( 0.6666667f, 1.0f - 0.25f) },
		{ XMFLOAT3(box_size, -box_size*squash_y, -box_size),   XMFLOAT2( 0.3333333f, 1.0f - 0.0f) },
		//10
		{ XMFLOAT3(box_size, box_size*squash_y, box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.25f) }, // - x
		{ XMFLOAT3(box_size, -box_size*squash_y, box_size),  XMFLOAT2( 0.3333333f, 1.0f - 0.25f) },
		{ XMFLOAT3(box_size, -box_size*squash_y, -box_size),   XMFLOAT2( 0.3333333f, 1.0f - 0.0f) },
		//11
		{ XMFLOAT3(-box_size, box_size*squash_y, -box_size), XMFLOAT2( 0.6666667f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(-box_size, -box_size*squash_y, -box_size),  XMFLOAT2( 0.3333333f, 1.0f - 0.75f) },
		{ XMFLOAT3(-box_size, box_size*squash_y, box_size),   XMFLOAT2( 0.6666667f, 1.0f - 0.5f) },
		//12
		{ XMFLOAT3(-box_size, -box_size*squash_y, -box_size), XMFLOAT2( 0.3333333f, 1.0f - 0.75f) }, // - x
		{ XMFLOAT3(-box_size, -box_size*squash_y, box_size),  XMFLOAT2( 0.3333333f, 1.0f - 0.5f) },
		{ XMFLOAT3(-box_size, box_size*squash_y, box_size),   XMFLOAT2( 0.6666667f, 1.0f - 0.5f) },
	};


	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = cubeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			&m_vertexBuffer
			)
		);

	// Load mesh indices. Each trio of indices represents
	// a triangle to be rendered on the screen.
	// For example: 0,2,1 means that the vertices with indexes
	// 0, 2 and 1 from the vertex buffer compose the 
	// first triangle of this mesh.
	static const unsigned short cubeIndices[] =
	{
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11,
		12,13,14,
		15,16,17,
		18,19,20,
		21,22,23,
		24,25,26,
		27,28,29,
		30,31,32,
		33,34,35,



	};

	m_indexCount = ARRAYSIZE(cubeIndices);

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = cubeIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&m_indexBuffer
			)
		);




	m_SkyCubeMap = new RenderCube();
	m_SkyCubeMap->Initialize(m_deviceResources, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);
	//m_SkyCubeMap->ClearTargets();

	m_SkyPlaneCubeMap = new RenderCube();
	m_SkyPlaneCubeMap->Initialize(m_deviceResources, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);




	m_loadingComplete = true;

	
}




void Sky::ReleaseTexture()
{
#ifndef _DEBUG
	if (m_Texture != nullptr)
	{
		m_Texture->Release();
		
		m_Texture = nullptr;
	}
#endif
}


void Sky::LoadSkybox(char* f_name)
{
	wchar_t texture_filename[40];
	wchar_t sky_filename[30];

	//if (level == 1)
	//	m_model->RotateY(3.141);

	ReleaseTexture();
	size_t conv;
	mbstowcs_s(&conv,sky_filename, f_name, 30);

	swprintf_s(texture_filename, 40, L"%s.dds", sky_filename);

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename, nullptr, &m_Texture, MAXSIZE_T);

	//m_Texture = m_Resources->m_Textures->LoadTexture(f_name);

	bEnvironmentCreated = false;
}

void Sky::LoadLevel(int level)
{
	wchar_t texture_filename[40];
	wchar_t sky_filename[30];

	//if (level == 1)
	//	m_model->RotateY(3.141);
#ifndef _DEBUG
	if (m_Texture != nullptr)
		m_Texture->Release();
#endif
	//mbstowcs(sky_filename, skybox, 30);



	//swprintf(texture_filename, 40, L"Assets/Skyboxes/Low/%s.dds", sky_filename);

	//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename, nullptr, &m_Texture, MAXSIZE_T);

	m_Texture = m_Resources->m_Textures->LoadTexture(skybox);

}


void Sky::SetBuffers(Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer, UINT indexCount)
{
	m_vertexBuffer = vertexBuffer;
	m_indexBuffer = indexBuffer;
	m_indexCount = indexCount;
}


// Renders one frame using the vertex and pixel shaders.
void Sky::RenderCenter()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	XMFLOAT3 f_3 = m_Resources->m_Camera->GetEye();

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Matrix._14 = 0.0f;
	m_Matrix._24 = 0.0f;
	m_Matrix._34 = 0.0f;
	m_Resources->m_Camera->m_constantBufferData.model = m_Matrix;
	m_Resources->m_Camera->UpdateConstantBuffer();

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());

	// Prepare the constant buffer to send it to the graphics device.
	/*
	context->UpdateSubresource1(
	m_constantBuffer.Get(),
	0,
	NULL,
	&m_constantBufferData,
	0,
	0,
	0
	);
	*/
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionTex);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);

	//m_Resources->m_Camera->SetCloseProjection();
}



// Renders one frame using the vertex and pixel shaders.
void Sky::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	XMFLOAT3 f_3 = m_Resources->m_Camera->GetEye();

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Matrix._14 = f_3.x;
	m_Matrix._24 = f_3.y+ pos_y;
	m_Matrix._34 = f_3.z;
	m_Resources->m_Camera->m_constantBufferData.model = m_Matrix;
	m_Resources->m_Camera->UpdateConstantBuffer();

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());

	// Prepare the constant buffer to send it to the graphics device.
	/*
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
		);
		*/
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionTex);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);

	//m_Resources->m_Camera->SetCloseProjection();
}

Sky::~Sky(void)
{
}
