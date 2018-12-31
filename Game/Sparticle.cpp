#include "pch.h"
#include "Sparticle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;



bool Sparticle::InitializeBuffers( bool _bInstanced)
{
	int i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData, instanceData;
	HRESULT result;

	bInstanced = _bInstanced;

	// Set the maximum number of vertices in the vertex array.
	m_vertexCount = m_maxParticles * 4;

	// Set the maximum number of indices in the index array.
	m_indexCount = m_maxParticles * 6;

	// Create the vertex array for the particles that will be rendered.
	m_vertices = new VertexPositionTexCol[m_vertexCount];

	if (!m_vertices)
	{
		return false;
	}



	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexPositionTexCol) * m_vertexCount));



	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexPositionTexCol) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	result = m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	if (bInstanced == true)
	{

		m_instances.resize(MAX_ANY_PARTICLES);


		// Set up the description of the instance buffer.
		instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		instanceBufferDesc.ByteWidth = sizeof(ParticleInstance) * m_instances.size();
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		instanceBufferDesc.MiscFlags = 0;
		instanceBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the instance data.
		instanceData.pSysMem = m_instances.data();
		instanceData.SysMemPitch = 0;
		instanceData.SysMemSlicePitch = 0;

		// Create the instance buffer.
		result = m_deviceResources->GetD3DDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
		if (FAILED(result))
		{
			return false;
		}

	}

	// Release the index array since it is no longer needed.
	//delete[] indices;
	//indices = 0;

	return true;
}


Sparticle::~Sparticle(void)
{


}

Sparticle::Sparticle(AllResources* p_Resources)
{
	m_Resources = p_Resources;

	num_of_textures = 1;

	bInstanced = false;

	m_deviceResources = m_Resources->m_deviceResources;
}


void Sparticle::Render()
{
	XMStoreFloat4x4(&m_Resources->m_Camera->m_constantBufferData.model, XMMatrixIdentity());
	
	 //= m_Resources->m_Camera->flat_model_matrix;
	m_Resources->m_Camera->UpdateConstantBuffer();
	//m_Resources->m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(constantBuffer.Get(), 0, NULL, constantBufferData, 0, 0);

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	if(num_of_textures>1)
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_Texture2.GetAddressOf());
	if (num_of_textures>2)
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(2, 1, m_Texture3.GetAddressOf());

	if (bInstanced == true)
	{
		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];


		// Set the buffer strides.
		strides[0] = sizeof(VertexPositionTexCol);
		strides[1] = sizeof(ParticleInstance);

		// Set the buffer offsets.
		offsets[0] = 0;
		offsets[1] = 0;

		// Set the array of pointers to the vertex and instance buffers.
		bufferPointers[0] = m_vertexBuffer;
		bufferPointers[1] = m_instanceBuffer;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

		m_Resources->m_deviceResources->GetD3DDeviceContext()->DrawInstanced(m_vertexCount, m_instances.size(), 0, 0);
	}
	else
	{
		unsigned int stride;
		unsigned int offset;

		// Set vertex buffer stride and offset.
		stride = sizeof(VertexPositionTexCol);
		offset = 0;

		m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	
		m_Resources->m_deviceResources->GetD3DDeviceContext()->DrawIndexed(
			m_indexCount,
			0,
			0
		);
	}
	
}



bool Sparticle::UpdateInstances(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;

	ParticleInstance* instancePtr;



	// Lock the vertex buffer.
	result = deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	instancePtr = (ParticleInstance*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(instancePtr, m_instances.data(), (sizeof(ParticleInstance) * (m_instances.size())));

	// Unlock the vertex buffer.
	deviceContext->Unmap(m_instanceBuffer, 0);


	return true;
}


void Sparticle::GetLightAtPosition(DirectX::XMFLOAT3 &pos, DirectX::XMFLOAT4 &light)
{
	light.x = m_Resources->m_Lights->m_lightBufferData.ambientColor.x*0.4f;// *DirectX::XMFLOAT4(0.2, 0.2f, 0.2f, 0.2f);
	light.y = m_Resources->m_Lights->m_lightBufferData.ambientColor.y*0.4f;// *DirectX::XMFLOAT4(0.2, 0.2f, 0.2f, 0.2f);
	light.z = m_Resources->m_Lights->m_lightBufferData.ambientColor.z*0.4f;// *DirectX::XMFLOAT4(0.2, 0.2f, 0.2f, 0.2f);
	light.x += m_Resources->m_Lights->m_lightBufferData.diffuseColor.x *0.2f;
	light.y += m_Resources->m_Lights->m_lightBufferData.diffuseColor.y *0.2f;
	light.z += m_Resources->m_Lights->m_lightBufferData.diffuseColor.z *0.2f;
	light.w = 1.0f;

	for (int j = 0; j < m_Resources->m_Lights->m_spotLights.size(); j++)
	{
		if (m_Resources->m_Camera->Within3DManhattanDistance(m_Resources->m_Lights->m_spotLights[j].pos.x, m_Resources->m_Lights->m_spotLights[j].pos.y, m_Resources->m_Lights->m_spotLights[j].pos.z,
			pos.x, pos.y, pos.z, m_Resources->m_Lights->m_spotLights[j].radius) > 0.0f) // check manhattan cube
		{
			XMVECTOR l = (XMLoadFloat3(&m_Resources->m_Lights->m_spotLights[j].pos) - XMLoadFloat3(&XMFLOAT3(pos.x, pos.y, pos.z)));// / m_Resources->m_Lights->m_pointLights[j].radius;
			float len;
			XMStoreFloat(&len, XMVector3Length(l));
			if (len < m_Resources->m_Lights->m_spotLights[j].radius)
			{
				float atten = (1.0f - (len / m_Resources->m_Lights->m_spotLights[j].radius)) * (1.0 / (1.0 + 0.1*len + 0.01*len*len)); // 1.0f - (len / m_Resources->m_Lights->m_pointLights[j].radius);
				float attenB;

				l = XMVector3Normalize(l);

				XMVECTOR ldif = XMLoadFloat4(&m_Resources->m_Lights->m_spotLights[j].diffuse);
				XMVECTOR lamb = XMLoadFloat4(&m_Resources->m_Lights->m_spotLights[j].ambient);
				XMVECTOR ldir = XMLoadFloat3(&m_Resources->m_Lights->m_spotLights[j].dir);

				XMVECTOR spot = XMLoadFloat(&m_Resources->m_Lights->m_spotLights[j].spot);

				//float spotlight = (pow(max(dot(-l, lights[i].dir), 0.0f), lights[i].spot)*lightIntensity);
				//output.color += spotlight*atten*((lights[i].diffuse + lights[i].ambient)*2.0f);

				float nil = 0.0f;
				XMVECTOR vattenB = XMVectorPow(XMVectorMax(XMVector3Dot(-l, ldir), XMLoadFloat(&nil)), spot); // intensity1
				XMStoreFloat(&attenB, vattenB);

				atten = (atten*(attenB*1.0f));
				if (atten > 0.0f)
				{
					XMVECTOR out = (((ldif*0.75f) + lamb)*0.75f)*atten;
					XMFLOAT4 light_col;
					XMStoreFloat4(&light_col, out);
					light.x = max(light.x, light_col.x);
					light.y = max(light.y, light_col.y);
					light.z = max(light.z, light_col.z);
				}
			}
		}
	}


	for (int j = 0; j < m_Resources->m_Lights->m_pointLights.size(); j++)
	{
		if (m_Resources->m_Camera->Within3DManhattanDistance(m_Resources->m_Lights->m_pointLights[j].pos.x, m_Resources->m_Lights->m_pointLights[j].pos.y, m_Resources->m_Lights->m_pointLights[j].pos.z,
			pos.x, pos.y, pos.z, m_Resources->m_Lights->m_pointLights[j].radius) > 0.0f) // check manhattan cube
		{
			{
				XMVECTOR l = (XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[j].pos) - XMLoadFloat3(&XMFLOAT3(pos.x, pos.y, pos.z)));// / m_Resources->m_Lights->m_pointLights[j].radius;
				float len;
				XMStoreFloat(&len, XMVector3Length(l));
				if (len < m_Resources->m_Lights->m_pointLights[j].radius)
				{
					float dot;
					XMStoreFloat(&dot, XMVector3Dot(l, l));
					float atten = (1.0f - (len / m_Resources->m_Lights->m_pointLights[j].radius)) * (1.0 / (1.0 + 0.1*len + 0.01*len*len)); // 1.0f - (len / m_Resources->m_Lights->m_pointLights[j].radius);



					XMVECTOR ldif = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].diffuse);
					XMVECTOR lamb = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].ambient);



					XMVECTOR out = (((ldif*0.75f) + ldif*0.5f)*0.5f)*atten;
					XMFLOAT4 light_col;
					XMStoreFloat4(&light_col, out);
					light.x = max(light.x, light_col.x);
					light.y = max(light.y, light_col.y);
					light.z = max(light.z, light_col.z);

					//col.w += light_col.w;
				}
			}
		}
	}
}


bool Sparticle::UpdateVertecies(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;


	if (bInstanced == true)
		UpdateInstances(deviceContext);



	VertexPositionTexCol* verticesPtr;

	// Lock the vertex buffer.
	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexPositionTexCol*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexPositionTexCol) * (m_vertexCount)));

	// Unlock the vertex buffer.
	deviceContext->Unmap(m_vertexBuffer, 0);


	return true;
}

