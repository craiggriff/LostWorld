////////////////////////////////////////////////////////////////////////////////
// Filename: skyplaneclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "skyplaneclass.h"


using namespace Game;
using namespace DirectX;
using namespace std;




SkyPlaneClass::SkyPlaneClass(AllResources* pm_Resources)
{
	m_Resources = pm_Resources;

	m_deviceResources = m_Resources->m_deviceResources;
	//m_CloudTexture = nullptr;
	//m_PerturbTexture = nullptr;



}


SkyPlaneClass::~SkyPlaneClass()
{


}


bool SkyPlaneClass::Initialize()
{
	int skyPlaneResolution, textureRepeat;
	float skyPlaneWidth, skyPlaneTop, skyPlaneBottom;
	bool result;


	// Set the sky plane parameters.
	skyPlaneResolution = 36;
	skyPlaneWidth = 5000.0f;
	skyPlaneTop = 100.5f;
	skyPlaneBottom = -300.5f;
	textureRepeat = 8;

	// Set the sky plane shader related parameters.
	m_scale = 0.3f;
	m_brightness = 0.5f;

	// Initialize the translation to zero.
	m_translationx = 0.0f;
	m_translationz = 0.0f;

	// Create the sky plane.
	result = InitializeSkyPlane(skyPlaneResolution, skyPlaneWidth, skyPlaneTop, skyPlaneBottom, textureRepeat);
	if(!result)
	{
		return false;
	}

	// Create the vertex and index buffer for the sky plane.
	result = InitializeBuffers(m_deviceResources->GetD3DDevice(), skyPlaneResolution);
	if(!result)
	{
		return false;
	}

	// Setup the cloud translation speed increments.
	m_translationSpeed[0] = 0.003f;   // First texture X translation speed.
	m_translationSpeed[1] = 0.0f;      // First texture Z translation speed.
	m_translationSpeed[2] = 0.0015f;  // Second texture X translation speed.
	m_translationSpeed[3] = 0.0f;      // Second texture Z translation speed.

	

	// Initialize the texture translation values.
	m_textureTranslation[0] = 0.0f;
	m_textureTranslation[1] = 0.0f;
	m_textureTranslation[2] = 0.0f;
	m_textureTranslation[3] = 0.0f;



	m_CloudTexture = m_Resources->m_Textures->LoadTexture("cloud001");
	m_PerturbTexture = m_Resources->m_Textures->LoadTexture("perturb001");



	return true;
}


void SkyPlaneClass::Shutdown()
{
	// Release the sky plane textures.
	ReleaseTextures();

	// Release the vertex and index buffer that were used for rendering the sky plane.
	ShutdownBuffers();

	// Release the sky plane array.
	ShutdownSkyPlane();

	return;
}


void SkyPlaneClass::RenderCenter()
{

	XMFLOAT4X4 m_Matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT3 f_3 = m_Resources->m_Camera->GetEye();

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Matrix._14 = 0.0f;
	m_Matrix._24 = -70.0f;
	m_Matrix._34 = 0.0f;


	m_Resources->m_Camera->m_constantBufferData.model = m_Matrix;
	m_Resources->m_Camera->UpdateConstantBuffer();



	// Render the sky plane.
	RenderBuffers(m_deviceResources->GetD3DDeviceContext());



	m_Resources->SetSkyplaneShader();

	m_SkyplaneBufferData.translationx = m_translationx;
	m_SkyplaneBufferData.translationz = m_translationz;
	m_SkyplaneBufferData.scale = 1.0f;
	m_SkyplaneBufferData.brightness = 0.5f;


	m_Resources->UpdateSkyplaneBuffer(&m_SkyplaneBufferData);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_CloudTexture.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_PerturbTexture.GetAddressOf());

	// Render the triangles.
	m_deviceResources->GetD3DDeviceContext()->DrawIndexed(m_indexCount, 0, 0);

	return;
}



void SkyPlaneClass::Render()
{

	XMFLOAT4X4 m_Matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT3 f_3 = m_Resources->m_Camera->GetEye();

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Matrix._14 = f_3.x;
	m_Matrix._24 = f_3.y + 2.0f;
	m_Matrix._34 = f_3.z;


	m_Resources->m_Camera->m_constantBufferData.model = m_Matrix;
	m_Resources->m_Camera->UpdateConstantBuffer();



	// Render the sky plane.
	RenderBuffers(m_deviceResources->GetD3DDeviceContext());



	m_Resources->SetSkyplaneShader();

	m_SkyplaneBufferData.translationx = m_translationx;
	m_SkyplaneBufferData.translationz = m_translationz;
	m_SkyplaneBufferData.scale = 1.0f;
	m_SkyplaneBufferData.brightness = 0.5f;


	m_Resources->UpdateSkyplaneBuffer(&m_SkyplaneBufferData);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_CloudTexture.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_PerturbTexture.GetAddressOf());

	// Render the triangles.
	m_deviceResources->GetD3DDeviceContext()->DrawIndexed(m_indexCount, 0, 0);

	return;
}


void SkyPlaneClass::Update(float timeTotal, float timeDelta)
{
	// Increment the texture translation value each frame.

	m_translationspeedx = -m_Resources->m_LevelInfo.wind.x*0.00003f;
	m_translationspeedz = -m_Resources->m_LevelInfo.wind.z*0.00003f;

	m_translationx += m_translationspeedx;
	if (m_translationx > 1.0f)
	{
		m_translationx -= 1.0f;
	}
	else
	{
		if (m_translationx < 0.0f)
		{
			m_translationx += 1.0f;
		}
	}


	m_translationz += m_translationspeedz;
	if (m_translationz > 1.0f)
	{
		m_translationz -= 1.0f;
	}
	else
	{
		if (m_translationz < 0.0f)
		{
			m_translationz += 1.0f;
		}
	}

	
	// Increment the translation values to simulate the moving clouds.
	m_textureTranslation[0] += m_translationSpeed[0];
	m_textureTranslation[1] += m_translationSpeed[1];
	m_textureTranslation[2] += m_translationSpeed[2];
	m_textureTranslation[3] += m_translationSpeed[3];

	// Keep the values in the zero to one range.
	if (m_textureTranslation[0] > 1.0f) { m_textureTranslation[0] -= 1.0f; }
	if (m_textureTranslation[1] > 1.0f) { m_textureTranslation[1] -= 1.0f; }
	if (m_textureTranslation[2] > 1.0f) { m_textureTranslation[2] -= 1.0f; }
	if (m_textureTranslation[3] > 1.0f) { m_textureTranslation[3] -= 1.0f; }

	return;
}




int SkyPlaneClass::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* SkyPlaneClass::GetCloudTexture()
{
	return m_CloudTexture.Get();
}


ID3D11ShaderResourceView* SkyPlaneClass::GetPerturbTexture()
{
	return m_PerturbTexture.Get();
}


float SkyPlaneClass::GetScale()
{
	return m_scale;
}


float SkyPlaneClass::GetBrightness()
{

	return m_brightness;
}




bool SkyPlaneClass::InitializeSkyPlane(int skyPlaneResolution, float skyPlaneWidth, float skyPlaneTop, float skyPlaneBottom, int textureRepeat)
{
	float quadSize, radius, constant, textureDelta;
	int i, j, index;
	float positionX, positionY, positionZ, tu, tv;


	// Create the array to hold the sky plane coordinates.
	m_skyPlane = new SkyPlaneType[(skyPlaneResolution + 1) * (skyPlaneResolution + 1)];
	if(!m_skyPlane)
	{
		return false;
	}

	// Determine the size of each quad on the sky plane.
	quadSize = skyPlaneWidth / (float)skyPlaneResolution;

	// Calculate the radius of the sky plane based on the width.
	radius = skyPlaneWidth / 2.0f;

	// Calculate the height constant to increment by.
	constant = (skyPlaneTop - skyPlaneBottom) / (radius * radius);

	// Calculate the texture coordinate increment value.
	textureDelta = (float)textureRepeat / (float)skyPlaneResolution;

	// Loop through the sky plane and build the coordinates based on the increment values given.
	for(j=0; j<=skyPlaneResolution; j++)
	{
		for(i=0; i<=skyPlaneResolution; i++)
		{
			// Calculate the vertex coordinates.
			positionX = (-0.5f * skyPlaneWidth) + ((float)i * quadSize);
			positionZ = (-0.5f * skyPlaneWidth) + ((float)j * quadSize);
			positionY = skyPlaneTop - (constant * ((positionX * positionX) + (positionZ * positionZ)));

			// Calculate the texture coordinates.
			tu = (float)i * textureDelta;
			tv = (float)j * textureDelta;

			// Calculate the index into the sky plane array to add this coordinate.
			index = j * (skyPlaneResolution + 1) + i;

			float radius_full = (skyPlaneWidth)* 0.5f;
			float radius_fade = radius_full *0.9f;

			float radius_diff = radius_full - radius_fade;

			float alpha_level=0.0f;
			
			float len = btVector3(positionX, 0.0f, positionZ).length();

			if (len > radius_fade)
			{
				if (len < radius_full)
				{
					alpha_level = 1.0f;  //alpha_level = (1.0f / (radius_full - radius_fade) ) *  (len - radius_fade);
				}
				else
				{
					alpha_level = 0.0f;
				}
			}
			else
			{
				alpha_level = 1.0f;
			}
			//alpha_level = alpha_level;

			//float alph = 
			//alpha_level = 0.5f;
			// Add the coordinates to the sky plane array.
			m_skyPlane[index].x = positionX;
			m_skyPlane[index].y = positionY;
			m_skyPlane[index].z = positionZ;
			m_skyPlane[index].tu = tu;
			m_skyPlane[index].tv = tv;
			m_skyPlane[index].aa = alpha_level;
		}
	}

	return true;
}


void SkyPlaneClass::ShutdownSkyPlane()
{
	// Release the sky plane array.
	if(m_skyPlane)
	{
		delete [] m_skyPlane;
		m_skyPlane = 0;
	}

	return;
}


bool SkyPlaneClass::InitializeBuffers(ID3D11Device* device, int skyPlaneResolution)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i, j, index, index1, index2, index3, index4;


	// Calculate the number of vertices in the sky plane mesh.
	m_vertexCount = (skyPlaneResolution + 1) * (skyPlaneResolution + 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;
		
	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index into the vertex array.
	index = 0;

	// Load the vertex and index array with the sky plane array data.
	for(j=0; j<skyPlaneResolution; j++)
	{
		for(i=0; i<skyPlaneResolution; i++)
		{
			index1 = j * (skyPlaneResolution + 1) + i;
			index2 = j * (skyPlaneResolution + 1) + (i+1);
			index3 = (j+1) * (skyPlaneResolution + 1) + i;
			index4 = (j+1) * (skyPlaneResolution + 1) + (i+1);

			// Triangle 1 - Upper Left
			vertices[index].position = XMFLOAT3(m_skyPlane[index1].x, m_skyPlane[index1].y, m_skyPlane[index1].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index1].tu, m_skyPlane[index1].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index1].aa, m_skyPlane[index1].ab);
			indices[index] = index;
			index++;

			// Triangle 1 - Bottom Left
			vertices[index].position = XMFLOAT3(m_skyPlane[index3].x, m_skyPlane[index3].y, m_skyPlane[index3].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index3].tu, m_skyPlane[index3].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index3].aa, m_skyPlane[index3].ab);
			indices[index] = index;
			index++;

			// Triangle 1 - Upper Right
			vertices[index].position = XMFLOAT3(m_skyPlane[index2].x, m_skyPlane[index2].y, m_skyPlane[index2].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index2].tu, m_skyPlane[index2].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index2].aa, m_skyPlane[index2].ab);
			indices[index] = index;
			index++;



			// Triangle 2 - Bottom Left
			vertices[index].position = XMFLOAT3(m_skyPlane[index3].x, m_skyPlane[index3].y, m_skyPlane[index3].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index3].tu, m_skyPlane[index3].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index3].aa, m_skyPlane[index3].ab);
			indices[index] = index;
			index++;

			// Triangle 2 - Bottom Right
			vertices[index].position = XMFLOAT3(m_skyPlane[index4].x, m_skyPlane[index4].y, m_skyPlane[index4].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index4].tu, m_skyPlane[index4].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index4].aa, m_skyPlane[index4].ab);
			indices[index] = index;
			index++;

			// Triangle 2 - Upper Right
			vertices[index].position = XMFLOAT3(m_skyPlane[index2].x, m_skyPlane[index2].y, m_skyPlane[index2].z);
			vertices[index].texture = XMFLOAT2(m_skyPlane[index2].tu, m_skyPlane[index2].tv);
			vertices[index].alpha = XMFLOAT2(m_skyPlane[index2].aa, m_skyPlane[index2].ab);
			indices[index] = index;
			index++;


		}
	}

	// Set up the description of the vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void SkyPlaneClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void SkyPlaneClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
    stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}





void SkyPlaneClass::ReleaseTextures()
{
	// Release the texture objects.
	if(m_PerturbTexture)
	{
		//m_PerturbTexture->Shutdown();
		//delete m_PerturbTexture;
		//m_PerturbTexture = 0;
	}

	if(m_CloudTexture)
	{
		//m_CloudTexture->Shutdown();
		//delete m_CloudTexture;
		//m_CloudTexture = 0;
	}

	return;
}