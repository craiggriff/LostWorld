#include "pch.h"
#include "Modicle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;


int ModSortCB(const VOID* arg1, const VOID* arg2)
{
	part_index* p1 = (part_index*)arg1;
	part_index* p2 = (part_index*)arg2;

	if (p1->dist == p2->dist)
	{
		return 0;
	}
	else
	{
		if (p1->dist<p2->dist)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}




bool Modicle::InitializeBuffers()
{
	int i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Set the maximum number of vertices in the vertex array.
	m_vertexCount = m_maxParticles * p_ModelLoader->m_verticesCount;

	// Set the maximum number of indices in the index array.
	m_indexCount = m_maxParticles * p_ModelLoader->m_indicesCount;

	// Create the vertex array for the particles that will be rendered.
	m_vertices = new VertexData[m_vertexCount];

	if (!m_vertices)
	{
		return false;
	}

	// Create the index array.
	m_indices = new unsigned long[m_indexCount];
	if (!m_indices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexData) * m_vertexCount));


	// Initialize the index array.
	for (i = 0; i<m_maxParticles; i++)
	{
		for (j = 0; j < m_modelindexCount; j++)
		{
			m_indices[(i*m_modelindexCount) + j] = m_model_indices[j] + (i*m_modelvertexCount);
		}
	}

	// Initialize the vertex array.
	for (i = 0; i<m_maxParticles; i++)
	{
		for (j = 0; j < m_modelvertexCount; j++)
		{
			m_vertices[(i*m_modelvertexCount) + j] = m_model_vertices[j];
		}
	}


	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexData) * m_vertexCount;
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

	// Set up the description of the static index buffer.

	if (bSorting == true)
	{
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else
	{
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	if (bSorting == true)
	{
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		indexBufferDesc.CPUAccessFlags = 0;
	}
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the index array since it is no longer needed.
	//delete[] indices;
	//indices = 0;

	return true;
}


Modicle::~Modicle(void)
{


}



void Modicle::Reset()
{
	m_indexCount = 0;
	m_vertexCount = 0;
	total_count = 0;
}

void Modicle::SetCamera(float x, float y, float z)
{
	cam_pos = XMFLOAT3(x, y, z);

}

void Modicle::AddOne(DirectX::XMFLOAT4X4* matr, DirectX::XMFLOAT4* _cols)
{
	int i;
	XMVECTOR pvec;
	XMFLOAT3 vec;
	XMMATRIX mat = XMLoadFloat4x4(matr);

	if (total_count >= m_maxParticles)
		return;

	for (i = 0; i < m_modelvertexCount; i++)
	{
		pvec = XMLoadFloat3(&m_model_vertices[i].pos);
		pvec = XMVector3Transform(pvec, mat);
		XMStoreFloat3(&m_vertices[(total_count*m_modelvertexCount) + i].pos, pvec);
		m_vertices[(total_count*m_modelvertexCount) + i].pos.x += matr->_14;
		m_vertices[(total_count*m_modelvertexCount) + i].pos.y += matr->_24;
		m_vertices[(total_count*m_modelvertexCount) + i].pos.z += matr->_34;

		if (_cols != nullptr)
		{
			//m_vertices[(total_count*m_modelvertexCount) + i].col.w = m_model_vertices[i].col.w + _cols[i].w;
			//m_vertices[(total_count*m_modelvertexCount) + i].col.x = m_model_vertices[i].col.x + _cols[i].x;
			//m_vertices[(total_count*m_modelvertexCount) + i].col.y = m_model_vertices[i].col.y + _cols[i].y;
			//m_vertices[(total_count*m_modelvertexCount) + i].col.z = m_model_vertices[i].col.z + _cols[i].z;
		}


		pvec = XMLoadFloat3(&m_model_vertices[i].norm);
		pvec = XMVector3Transform(pvec, mat);
		XMStoreFloat3(&m_vertices[(total_count*m_modelvertexCount) + i].norm, pvec);
	}

	m_vertexCount += m_modelvertexCount;

	if (bSorting == true)
	{
		for (i = 0; i < m_modelindexCount / 3; i++)
		{
			p_index[i].part_no = i;

			pvec = XMVector3Transform(tcenter[i], mat);
			XMStoreFloat3(&vec, pvec);
			vec.x += matr->_14;
			vec.y += matr->_24;
			vec.z += matr->_34;

			p_index[i].dist = sqrt((vec.x - cam_pos.x) * (vec.x - cam_pos.x) +
				(vec.y - cam_pos.y) * (vec.y - cam_pos.y) +
				(vec.z - cam_pos.z) * (vec.z - cam_pos.z));
		}

		qsort(p_index, m_modelindexCount / 3, sizeof(part_index), ModSortCB);

		// transfer the indexes
		for (i = 0; i < m_modelindexCount / 3; i++)
		{
			m_indices[(total_count*m_modelindexCount) + (i * 3)] = m_model_indices[(p_index[i].part_no * 3)] + (total_count*m_modelvertexCount);
			m_indices[(total_count*m_modelindexCount) + (i * 3) + 1] = m_model_indices[(p_index[i].part_no * 3) + 1] + (total_count*m_modelvertexCount);
			m_indices[(total_count*m_modelindexCount) + (i * 3) + 2] = m_model_indices[(p_index[i].part_no * 3) + 2] + (total_count*m_modelvertexCount);
		}
	}



	total_count++;


	m_indexCount += m_modelindexCount;
}



Modicle::Modicle(std::shared_ptr<DX::DeviceResources> pm_deviceResources, ModelLoader* pp_ModelLoader, int maximum, int _sort)
{
	int i;
	XMFLOAT3 cent;

	if (_sort == 0)
		bSorting = false;
	else
		bSorting = true;

	m_deviceResources = pm_deviceResources;

	p_ModelLoader = pp_ModelLoader;

	m_maxParticles = maximum;




	m_modelvertexCount = p_ModelLoader->m_verticesCount;

	m_modelindexCount = p_ModelLoader->m_indicesCount;

	m_model_vertices = new VertexData[m_modelvertexCount];

	m_model_indices = new unsigned long[m_modelindexCount];


	if (bSorting == true)
	{
		tcenter = new XMVECTOR[m_modelindexCount / 3];
		p_index = new part_index[m_modelindexCount / 3];
	}

	// copy model localy in array format
	for (i = 0; i < p_ModelLoader->m_verticesCount; i++)
	{
		//m_model_vertices[i].col = p_ModelLoader->m_vertices[i].col;
		m_model_vertices[i].norm = p_ModelLoader->m_vertices[i].norm;
		m_model_vertices[i].pos = p_ModelLoader->m_vertices[i].pos;
		//m_model_vertices[i].specular = p_ModelLoader->m_vertices[i].specular;
		m_model_vertices[i].tex = p_ModelLoader->m_vertices[i].tex;
		//m_model_vertices[i].blend = p_ModelLoader->m_vertices[i].blend;
	}

	for (i = 0; i < p_ModelLoader->m_indicesCount; i++)
	{
		m_model_indices[i] = p_ModelLoader->m_indices[i];
	}
	if (bSorting == true)
	{
		for (i = 0; i < m_modelindexCount; i += 3)
		{
			cent.x = m_model_vertices[m_model_indices[i]].pos.x;
			cent.y = m_model_vertices[m_model_indices[i]].pos.y;
			cent.z = m_model_vertices[m_model_indices[i]].pos.z;

			cent.x = cent.x + m_model_vertices[m_model_indices[i + 1]].pos.x;
			cent.y = cent.y + m_model_vertices[m_model_indices[i + 1]].pos.y;
			cent.z = cent.z + m_model_vertices[m_model_indices[i + 1]].pos.z;

			cent.x = cent.x + m_model_vertices[m_model_indices[i + 2]].pos.x;
			cent.y = cent.y + m_model_vertices[m_model_indices[i + 2]].pos.y;
			cent.z = cent.z + m_model_vertices[m_model_indices[i + 2]].pos.z;

			cent.x = cent.x * 0.3333f;
			cent.y = cent.y * 0.3333f;
			cent.z = cent.z * 0.3333f;

			tcenter[i / 3] = XMLoadFloat3(&cent);

		}
	}


	InitializeBuffers();

}


void Modicle::Render(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> deviceContext, Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer, ModelViewProjectionConstantBuffer* constantBufferData)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexData);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->DrawIndexed(
		m_indexCount,
		0,
		0
		);
}



bool Modicle::UpdateVertecies(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_MAPPED_SUBRESOURCE mappedResource2;
	HRESULT result;

	VertexData* verticesPtr;
	unsigned long* indicesPtr;


	// Lock the vertex buffer.
	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexData*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexData) * (m_vertexCount)));

	// Unlock the vertex buffer.
	deviceContext->Unmap(m_vertexBuffer, 0);

	if (bSorting == true)
	{
		// Lock the index buffer.
		result = deviceContext->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource2);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the index buffer.
		indicesPtr = (unsigned long*)mappedResource2.pData;

		// Copy the data into the index buffer.
		memcpy(indicesPtr, (void*)m_indices, (sizeof(unsigned long) * (m_indexCount)));

		// Unlock the index buffer.
		deviceContext->Unmap(m_indexBuffer, 0);
	}

	return true;
}
