	#include "pch.h"
#include "FlagNode.h"

//#include "DirectXHelper.h"

using namespace Game;
using namespace DirectX;
using namespace DX;





FlagNode::FlagNode(AllResources* p_Resources, int _bUpdateable) :
	m_Resources(p_Resources)
{
	m_deviceResources = m_Resources->m_deviceResources;
	if (_bUpdateable == 1)
	{
		bUpdateable = true;
	}
	else
	{
		bUpdateable = false;
	}

	bPhysicsObject = false;
}

void FlagNode::InitFlagNode(int xbp, int ybp, int _total_x, int _total_y)
{
	xblockpos = xbp;
	yblockpos = ybp;

	total_x = _total_x;
	total_y = _total_y;

	start_id = 9999;
	end_id = 0;
	bLoaded = false;

	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f));
}




bool FlagNode::CreateUpdatebleVertexBuffer()
{
	unsigned long* indices;
	int i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	DirectX::XMFLOAT3 pos[3]; // creating central points of triangles for sorting
	DirectX::XMFLOAT3 cent;


	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexData) * m_verticesCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	result = m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	SetIndexBuffer();

	return true;
}

bool FlagNode::UpdateUpdatebleVertexBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_MAPPED_SUBRESOURCE mappedResource2;

	HRESULT result;
	VertexData* verticesPtr;
	unsigned long* indicesPtr;

	// Lock the vertex buffer.
	result = m_deviceResources->GetD3DDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexData*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices.data(), (sizeof(VertexData) * (m_verticesCount)));

	// Unlock the vertex buffer.
	m_deviceResources->GetD3DDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
}


void FlagNode::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void FlagNode::setWorldTransform(const btTransform &worldTransform)
{
	m_initialTransform = worldTransform;

	m_rigidbody->setWorldTransform(worldTransform);
}



void FlagNode::Render(int complexity)
{
	UINT stride = sizeof(VertexData);
	UINT offset = 0;

	m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, GetVertexBuffer().GetAddressOf(), &stride, &offset);
	if (complexity == 1)
	{
		m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(GetIndexBufferSimple().Get(), DXGI_FORMAT_R16_UINT, 0);
	}
	else
	{
		m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
	}

	m_deviceResources->GetD3DDeviceContext()->DrawIndexed(m_indicesCount, 0, 0);
}

void FlagNode::GetMatrix(DirectX::XMFLOAT4X4* matr)
{
	*matr = m_constantBufferData.model;

}





void FlagNode::UpdateProjectionMatrix(XMMATRIX *projectionMatrix)
{
	//XMStoreFloat4x4(&m_constantBufferData.projection, *projectionMatrix);
}

void FlagNode::Update(XMMATRIX *viewMatrix, float timeTotal)
{
	XMStoreFloat4x4(&m_constantBufferData.view, *viewMatrix);
}


void FlagNode::SetPosition(float x, float y, float z)
{
	XMMATRIX m_modelMatrix;

	auto rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	auto translationMatrix = XMMatrixTranslation(x, y, z);
	auto scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	m_modelMatrix = XMMatrixTranspose((translationMatrix *  rotationMatrix)*scalingMatrix);

	XMStoreFloat4x4(&m_constantBufferData.model, m_modelMatrix);

	//m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(x, y, z));

}

bool FlagNode::UpdateFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols)
{
	int i, j;

	for (i = 0; i < ycoords; i++)
	{
		for (j = 0; j < xcoords; j++)
		{
			height_map[j][i] = whole_height_map[j + xblockpos][i + yblockpos];
			normals[j][i] = XMFLOAT3(whole_normals[j + xblockpos][i + yblockpos].x, whole_normals[j + xblockpos][i + yblockpos].y, whole_normals[j + xblockpos][i + yblockpos].z);// hm_info->normal[((j)+xblockpos + (((i)+yblockpos)*hm_info->FlagNodeWidth))];

			//cols[j][i] = p_cols[j + xblockpos][i + yblockpos];
			cols[j][i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			height_fields[j + (i*xcoords)] = height_map[i][j];
		}
	}
	UpdateBuffers();
	//CreateBuffers();

	if (bUpdateable == true)
	{
		UpdateUpdatebleVertexBuffer();
	}
	else
	{
		SetVertexBuffer();
	}

	return true;
}


bool FlagNode::LoadFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols, int xnum, int ynum, float scale, int xplane)
{
	int i, j;
	m_scale = scale;
	xcoords = xnum;
	ycoords = ynum;
	XMMATRIX m_modelMatrix;
	XMVECTOR data;
	XMVECTORF32 floatingVector = { 0.0f, 0.0f, 0.0f, 1.0f };
	data = floatingVector;

	no_phy_verticies = ((xnum)*(ynum)) * 8;
	height_fields = new float[xnum*ynum];
	m_phy_vertices = new VertexType[no_phy_verticies];

	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f));

	height_map = new float*[ynum];
	normals = new XMFLOAT3*[ynum];
	cols = new XMFLOAT4*[ynum];

	for (i = 0; i < ynum; ++i) {
		height_map[i] = new float[xnum];
		normals[i] = new XMFLOAT3[xnum];
		cols[i] = new XMFLOAT4[xnum];
	}

	for (i = 0; i < ynum; i++)
	{
		for (j = 0; j < xnum; j++)
		{
			height_map[j][i] = whole_height_map[j + xblockpos][i + yblockpos];
			normals[j][i] = XMFLOAT3(whole_normals[j + xblockpos][i + yblockpos].x, whole_normals[j + xblockpos][i + yblockpos].y, whole_normals[j + xblockpos][i + yblockpos].z);// hm_info->normal[((j)+xblockpos + (((i)+yblockpos)*hm_info->FlagNodeWidth))];
			cols[j][i] = p_cols[j + xblockpos][i + yblockpos];
			cols[j][i].w = 0.5f;
			height_fields[j + (i*xnum)] = height_map[i][j];
		}
	}

	CreateBuffers();

	if (bUpdateable == true)
	{
		CreateUpdatebleVertexBuffer();
	}
	else
	{
		SetVertexBuffer();
	}
	SetIndexBuffer();
	//SetIndexBufferSimple();

	return true;

}

void FlagNode::UpdateBuffers()
{
	int i, j;
	for (j = 0; j<ycoords; j++)
	{
		for (i = 0; i<xcoords; i++)
		{
			m_vertices[i + (j*xcoords)].pos.y = height_map[i][j];
			m_vertices[i + (j*xcoords)].norm.x = -normals[i][j].x;
			m_vertices[i + (j*xcoords)].norm.y = -normals[i][j].y;
			m_vertices[i + (j*xcoords)].norm.z = -normals[i][j].z;

			m_vertices[(ycoords * xcoords) + i + (j*xcoords)].pos.y = height_map[i][j];
			m_vertices[(ycoords * xcoords) + i + (j*xcoords)].norm.x = normals[i][j].x;
			m_vertices[(ycoords * xcoords) + i + (j*xcoords)].norm.y = normals[i][j].y;
			m_vertices[(ycoords * xcoords) + i + (j*xcoords)].norm.z = normals[i][j].z;
			
		}
	}
}

void FlagNode::CreateBuffers()
{
	int i, j, total_side_verts; // size of flag
	int end_dist;


	m_indices.clear();
	m_vertices.clear();
	m_indices_simple.clear();
	for (j = 0; j<ycoords; j++)
	{
		for (i = 0; i<xcoords; i++)
		{ 
			float x_size = (1.0f / float(total_x));

			float y_size = (1.0f / float(total_y));

			float u_value = (((1.0f / float(xcoords - 1))*float(i)));
			float v_value = (((1.0f / float(ycoords - 1))*float(j)));

			u_value = (u_value * x_size)+((xblockpos/8.0f)*x_size);
			v_value = 1.0f - (v_value * y_size)+((yblockpos/8.0f)*y_size);

			AddVertexTexNormCol(
				(float(i)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				height_map[i][j],
				(float(j)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				v_value,
				u_value,
				normals[i][j].x,
				normals[i][j].y,
				normals[i][j].z,
				cols[i][j].x,
				cols[i][j].y,
				cols[i][j].z,
				cols[i][j].w);
		}
	}

	total_side_verts = m_vertices.size();

	// back of flag verticies
	for (j = 0; j<ycoords; j++)
	{
		for (i = 0; i<xcoords; i++)
		{
			float x_size = (1.0f / float(total_x));

			float y_size = (1.0f / float(total_y));

			float u_value = (((1.0f / float(xcoords - 1))*float(i)));
			float v_value = (((1.0f / float(ycoords - 1))*float(j)));

			u_value = (u_value * x_size) + ((xblockpos / 8.0f)*x_size);
			v_value = ((v_value * y_size) + ((yblockpos / 8.0f)*y_size));

			AddVertexTexNormCol(
				(float(i)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				height_map[i][j],
				(float(j)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				v_value,
				u_value,
				normals[i][j].x,
				normals[i][j].y,
				normals[i][j].z,
				cols[i][j].x,
				cols[i][j].y,
				cols[i][j].z,
				cols[i][j].w);
		}
	}
	end_dist = 2;

	int cur_phy = 0;

	bool bHoles = false;

	// make simple indicies for distant FlagNode
	for (j = 0; j<ycoords - end_dist; j += 4)
	{
		for (i = 0; i<xcoords - end_dist; i += 4)
		{
			if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				if (j == 0)
				{
					// A
					m_indices_simple.push_back(i + (j*xcoords));
					m_indices_simple.push_back((i + 1) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));

					// B
					m_indices_simple.push_back((i + 1) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));

				}
				else
				{
					m_indices_simple.push_back(i + (j*xcoords));
					m_indices_simple.push_back((i + 2) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
				}
			}

			if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				// i == full
				if (i == 0)
				{
					// E
					m_indices_simple.push_back(i + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 1)*xcoords));

					// F
					m_indices_simple.push_back(i + ((j + 1)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 2)*xcoords));

					//m_indices_simple.push_back(i + (j*xcoords));
					//m_indices_simple.push_back((i + 2) + ((j + 1)*xcoords));
					//m_indices_simple.push_back(i + ((j + 2)*xcoords));

				}
				else
				{
					m_indices_simple.push_back(i + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 2)*xcoords));
				}
			}


			if (bHoles == false || !(m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				if (j == 0)
				{
					// C
					m_indices_simple.push_back((i + 3) + (j*xcoords));
					m_indices_simple.push_back((i + 4) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));

					// D
					m_indices_simple.push_back((i + 2) + (j*xcoords));
					m_indices_simple.push_back((i + 3) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
				}
				else
				{
					m_indices_simple.push_back((i + 2) + (j*xcoords));
					m_indices_simple.push_back((i + 4) + (j*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));

				}
			}

			if (bHoles == false || !(m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				// i = full
				if (i < (xcoords - end_dist) - 4)
				{
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + (j*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 2)*xcoords));
				}
				else
				{
					// I
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + (j*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 1)*xcoords));

					// J
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 1)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 2)*xcoords));

				}

			}


			if (bHoles == false || !(m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 4)*xcoords)].pos.y == 0.0f))
			{
				if (i == 0)
				{
					// G
					m_indices_simple.push_back(i + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 3)*xcoords));

					// H
					m_indices_simple.push_back(i + ((j + 3)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 4)*xcoords));
				}
				else
				{
					m_indices_simple.push_back(i + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back(i + ((j + 4)*xcoords));
				}
			}

			if (bHoles == false || !(m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 4)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 4)*xcoords)].pos.y == 0.0f))
			{

				if (j < (ycoords - end_dist) - 4)
				{
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 4)*xcoords));
					m_indices_simple.push_back(i + ((j + 4)*xcoords));
				}
				else
				{
					// K
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 4)*xcoords));
					m_indices_simple.push_back((i + 1) + ((j + 4)*xcoords));

					// L
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 1) + ((j + 4)*xcoords));
					m_indices_simple.push_back(i + ((j + 4)*xcoords));

				}

			}
			if (bHoles == false || !(m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + ((j + 4)*xcoords)].pos.y == 0.0f))
			{
				if (i < (xcoords - end_dist) - 4)
				{
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 4)*xcoords));
				}
				else
				{
					// M
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 3)*xcoords));

					// N
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 3)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 4)*xcoords));

					//m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					//m_indices_simple.push_back((i + 4) + ((j + 2)*xcoords));
					//m_indices_simple.push_back((i + 4) + ((j + 4)*xcoords));

				}
			}
			if (bHoles == false || !(m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 4) + ((j + 4)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 4)*xcoords)].pos.y == 0.0f))
			{
				if (j < (ycoords - end_dist) - 4)
				{
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 4)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 4)*xcoords));
				}
				else
				{
					// O
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 4) + ((j + 4)*xcoords));
					m_indices_simple.push_back((i + 3) + ((j + 4)*xcoords));

					// P
					m_indices_simple.push_back((i + 2) + ((j + 2)*xcoords));
					m_indices_simple.push_back((i + 3) + ((j + 4)*xcoords));
					m_indices_simple.push_back((i + 2) + ((j + 4)*xcoords));
				}
			}
		}
	}

	bHoles = false;


	for (j = 0; j<ycoords - end_dist; j += 2)
	{
		for (i = 0; i<xcoords - end_dist; i += 2)
		{
			if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back(i + (j*xcoords));
				m_indices.push_back((i + 1) + (j*xcoords));
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
			}

			if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 1)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back(i + (j*xcoords));
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back(i + ((j + 1)*xcoords));
			}


			if (bHoles == false || !(m_vertices[(i + 1) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back((i + 1) + (j*xcoords));
				m_indices.push_back((i + 2) + (j*xcoords));
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
			}

			if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 1)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back((i + 2) + (j*xcoords));
				m_indices.push_back((i + 2) + ((j + 1)*xcoords));
			}


			if (bHoles == false || !(m_vertices[i + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back(i + ((j + 1)*xcoords));
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back(i + ((j + 2)*xcoords));
			}

			if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back((i + 1) + ((j + 2)*xcoords));
				m_indices.push_back(i + ((j + 2)*xcoords));
			}
			if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back((i + 2) + ((j + 1)*xcoords));
				m_indices.push_back((i + 2) + ((j + 2)*xcoords));
			}
			if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 2)*xcoords)].pos.y == 0.0f))
			{
				m_indices.push_back((i + 1) + ((j + 1)*xcoords));
				m_indices.push_back((i + 2) + ((j + 2)*xcoords));
				m_indices.push_back((i + 1) + ((j + 2)*xcoords));
			}
		}
	}

	// create reverse flag
	{
		int ind_num = m_indices.size();
		for (i = 0; i < ind_num; i++)
		{
			m_indices.push_back(m_indices[ind_num - i - 1]+ total_side_verts);
		}
	}



	if (true)
	{
		// simple
		for (i = 0; i < m_indices_simple.size(); i++)
		{
			m_phy_vertices[cur_phy].x = m_vertices.at(m_indices_simple.at(i)).pos.x;
			m_phy_vertices[cur_phy].y = m_vertices.at(m_indices_simple.at(i)).pos.y;
			m_phy_vertices[cur_phy++].z = m_vertices.at(m_indices_simple.at(i)).pos.z;
		}
	}
	else
	{
		for (i = 0; i < m_indices.size(); i++)
		{
			m_phy_vertices[cur_phy].x = m_vertices.at(m_indices.at(i)).pos.x;
			m_phy_vertices[cur_phy].y = m_vertices.at(m_indices.at(i)).pos.y;
			m_phy_vertices[cur_phy++].z = m_vertices.at(m_indices.at(i)).pos.z;
		}
	}

	no_phy_verticies = cur_phy - 1;

	m_verticesCount = m_vertices.size();
	m_indicesCount = m_indices.size();
	m_indicesCount_simple = m_indices_simple.size();


}



void FlagNode::AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a)
{
	unique_ptr<VertexData> vd(new VertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	vd->SetColor(DirectX::XMFLOAT4(r, g, b, 0.0f));
	//vd->SetBlend(a);
	//vd->SetSpecular(0.0f);
	m_vertices.push_back(*vd);
}

void FlagNode::AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz)
{
	unique_ptr<VertexData> vd(new VertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	//vd->SetColor(DirectX::XMFLOAT4(rand_brightness, rand_brightness, rand_brightness, 1.0f));
	m_vertices.push_back(*vd);
}


void FlagNode::SetVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_vertices.data(), 0, 0 };
	UINT bytes = sizeof(VertexData) * m_verticesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_vertexBuffer));
}


void FlagNode::SetIndexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer));
}

void FlagNode::SetIndexBufferSimple()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices_simple.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount_simple;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer_simple));
}

FlagNode::~FlagNode(void)
{
	ClearMemory();
}


void FlagNode::ClearMemory()
{

	for (int i = 0; i < ycoords; ++i) {
		delete[] height_map[i];
	}
	delete[] height_map;
	//delete m_Texture;


	//m_box->m_model->Clearmemory();
	m_vertices.clear();
	m_indices.clear();
	m_indices_simple.clear();

	//m_rigidbody->~btRigidBody();
	//delete m_vertexBuffer.;
	//delete m_indexBuffer;

	//m_vertexBuffer->Release();
	//m_indexBuffer->Release();
	//m_indexBuffer_simple->Release();

	delete[] m_phy_vertices;

	//m_vertices.
}
