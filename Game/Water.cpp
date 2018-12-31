#include "pch.h"
#include "Water.h"
#include "PerlinNoise.h"
#include "SimplexNoise.h"


using namespace Game;
using namespace DirectX;
using namespace DX;


Water::Water(AllResources* p_Resources, int _bUpdateable) :
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
	m_rigidbody = nullptr;
	bPhysicsObject = false;



	bActive = true;
}

void Water::InitWater(int xbp, int ybp)
{
	xblockpos = xbp;
	yblockpos = ybp;

	start_id = 9999;
	end_id = 0;
	bLoaded = false;


	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f));
}




bool Water::CreateUpdatebleVertexBuffer()
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
	vertexBufferDesc.ByteWidth = sizeof(WaterVertexData) * m_verticesCount;
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

	

	return true;
}

bool Water::UpdateUpdatebleVertexBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_MAPPED_SUBRESOURCE mappedResource2;

	HRESULT result;
	WaterVertexData* verticesPtr;
	unsigned long* indicesPtr;

	// Lock the vertex buffer.
	result = m_deviceResources->GetD3DDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (WaterVertexData*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices.data(), (sizeof(WaterVertexData) * (m_verticesCount)));

	// Unlock the vertex buffer.
	m_deviceResources->GetD3DDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
}


void Water::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void Water::setWorldTransform(const btTransform &worldTransform)
{
	m_initialTransform = worldTransform;

	m_rigidbody->setWorldTransform(worldTransform);
}



void Water::Render(int complexity)
{
	if (bActive == false)return;

	UINT stride = sizeof(WaterVertexData);
	UINT offset = 0;
	DirectX::XMVECTOR dir; // diffuse dir
	DirectX::XMVECTOR dif; // diffuse col

	dir = XMLoadFloat3(&m_Resources->m_LevelInfo.diff_dir);



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

void Water::GetMatrix(DirectX::XMFLOAT4X4* matr)
{
	*matr = m_constantBufferData.model;

}



void Water::UpdateProjectionMatrix(XMMATRIX *projectionMatrix)
{
	//XMStoreFloat4x4(&m_constantBufferData.projection, *projectionMatrix);
}

void Water::Update(XMMATRIX *viewMatrix, float timeTotal)
{
	XMStoreFloat4x4(&m_constantBufferData.view, *viewMatrix);
}


void Water::SetPosition(float x, float y, float z)
{
	XMMATRIX m_modelMatrix;

	xpos = x;
	ypos = y;
	zpos = z;

	auto rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	auto translationMatrix = XMMatrixTranslation(x, y, z);
	auto scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	m_modelMatrix = XMMatrixTranspose((translationMatrix *  rotationMatrix)*scalingMatrix);

	XMStoreFloat4x4(&m_constantBufferData.model, m_modelMatrix);

	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(x, y, z));

}

void Water::GetActiveVerts(bool** water_active_map)
{
	int i, j;

	for (i = 0; i < ycoords; i++)
	{
		for (j = 0; j < xcoords; j++)
		{
			water_active_map[j + xblockpos][i + yblockpos]= b_ren[j][i];
		}

	}
}

bool Water::LoadTerrainHeight(float** whole_height_map)
{
	int i, j;
	XMVECTOR col;
	XMVECTOR nor;
	XMVECTOR lightIntensityA;
	DirectX::XMVECTOR dir; // diffuse dir
	DirectX::XMVECTOR dif; // diffuse col

	dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);
	dir = XMLoadFloat3(&m_Resources->m_LevelInfo.diff_dir);

	wave_height = m_Resources->m_LevelInfo.wind.y;
	x_mom = m_Resources->m_LevelInfo.wind.x;// (timeTotal*m_Resources->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Resources->m_LevelInfo.wind.z; // (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.4f;
	pnscale = m_Resources->m_LevelInfo.wind.w;

	for (i = 0; i < ycoords; i++)
	{
		for (j = 0; j < xcoords; j++)
		{
			land_dist[j][i] = whole_height_map[j + xblockpos][i + yblockpos];
			b_ren[j][i] = true;
			/*
			if (land_dist[j][i] > 1.0f)
				land_dist[j][i] = 1.0f;

			if (land_dist[j][i] < 0.0f)
				land_dist[j][i] = 0.0f;
				*/
		}
	}

	for (int i = 0; i < ycoords; i++)
	{
		for (int j = 0; j < xcoords; j++)
		{
			normals[j][i].x = 0.0f;
			normals[j][i].y = 1.0f;
			normals[j][i].z = 0.0f;

			cols[j][i] = m_Resources->m_LevelInfo.ambient_col;

			col = XMLoadFloat4(&cols[j][i]);
			nor = XMLoadFloat3(&XMFLOAT3(normals[j][i].x, normals[j][i].y, normals[j][i].z));

			lightIntensityA = XMVectorSaturate(XMVector3Dot(nor, -dir));
			float lightIntensity;
			XMStoreFloat(&lightIntensity, lightIntensityA);

			if (lightIntensity > 0.0f)
			{
				col = col + XMVectorSaturate(dif*lightIntensity);
				XMStoreFloat4(&cols[j][i], col);
			}
		}
	}
	MakeIndices();
	//SetIndexBuffer();;

	return true;
}

concurrency::task<void> Water::Update(float timeDelta, float timeTotal, float** water_height_map)
{

	return concurrency::create_task([timeTotal, this,water_height_map]
	{
		if (bActive == true)
		{
			XMVECTOR col;
			XMVECTOR nor;
			XMVECTOR lightIntensityA;
			//return;
			DirectX::XMVECTOR dir; // diffuse dir
			DirectX::XMVECTOR dif; // diffuse col

			dir = XMLoadFloat3(&m_Resources->m_LevelInfo.diff_dir);
			dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);

			float tl;
			float t;
			float tr;
			float r;
			float br;
			float b;
			float bl;
			float l;

			float scale_2 = 1.5f;

			for (int i = 0; i < ycoords; i++)
			{
				for (int j = 0; j < xcoords; j++)
				{
					if (b_ren[j][i] == true)
					{
						height_map[j][i] = water_height_map[j + xblockpos][i + yblockpos];
						//height_map[j][i] = (pn.noise(((float)(j + xblockpos) + x_movement)*pnscale, ((float)(i + yblockpos) + z_movement)*pnscale, change))*wave_height;
						//cols[j][i].w = (pn.noise(((float)(j + xblockpos) + x_movement)*pnscale, ((float)(i + yblockpos) + z_movement)*pnscale, change));// *wave_height;
						//height_map[j][i] += (pn.noise(((float)(j + xblockpos) + x_movement)*(pnscale*scale_2), ((float)(i + yblockpos) + z_movement)*(pnscale*scale_2), change*scale_2))*wave_height;
						height_fields[j + (i*xcoords)] = height_map[j][i];

						
						
						if (i + yblockpos > 0 && j + xblockpos > 0 && i + yblockpos < m_Resources->total_terrrain_y_points - 1 && j + xblockpos < m_Resources->total_terrrain_x_points - 1)
						{
							tl = water_height_map[j + xblockpos-1][i + yblockpos-1];
							t = water_height_map[j + xblockpos - 1][i + yblockpos];
							tr = water_height_map[j + xblockpos-1][i + yblockpos+1];
							r = water_height_map[j + xblockpos][i + yblockpos+1];
							br = water_height_map[j + xblockpos+1][i + yblockpos+1];
							b = water_height_map[j + xblockpos+1][i + yblockpos];
							bl = water_height_map[j + xblockpos+1][i + yblockpos-1];
							l = water_height_map[j + xblockpos][i + yblockpos-1];
						}
						else
						{

						}

						float dZ = -((tr + (1.5f * r) + br) - (tl + (1.5f * l) + bl));
						float dX = (tl + (1.5f * t) + tr) - (bl + (1.5f * b) + br);
						float dY = 35.5;// / pStrength;

						btVector3 v(dX, dY, dZ);
						v.normalize();


						normals[j][i].x = (float)v.getX();
						normals[j][i].y = (float)v.getY();
						normals[j][i].z = (float)v.getZ();
						/*
						normals[j][i].x = 0.0f;
						normals[j][i].y = 1.0f;
						normals[j][i].z = 0.0f;
						*/
						dY = 3.5f;
						v = btVector3(dX, dY, dZ);
						v.normalize();
						//cols[j][i].y = 1.0f -(float)v.getY();

						cols[j][i] = m_Resources->m_LevelInfo.ambient_col;

						col = XMLoadFloat4(&cols[j][i]);
						nor = XMLoadFloat3(&XMFLOAT3(normals[j][i].x, normals[j][i].y, normals[j][i].z));

						lightIntensityA = XMVectorSaturate(XMVector3Dot(nor, -dir));
						float lightIntensity;
						XMStoreFloat(&lightIntensity, lightIntensityA);

						if (lightIntensity > 0.0f)
						{
							col = col + XMVectorSaturate(dif*lightIntensity);
							XMStoreFloat4(&cols[j][i], col);
						}

						if (true)
						{
							XMVECTOR tan;

							XMVECTOR c1 = XMVector3Cross(nor, XMLoadFloat3(&DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f)));
							XMVECTOR c2 = XMVector3Cross(nor, XMLoadFloat3(&DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));

							float l1;
							float l2;
							XMStoreFloat(&l1, XMVector3Length(c1));
							XMStoreFloat(&l2, XMVector3Length(c2));

							if (l1 > l2)
							{
								tan = c1;
							}
							else
							{
								tan = c2;
							}

							tan = XMVector3Normalize(tan);

							XMVECTOR binormal = XMVector3Cross(nor, tan);
							binormal = XMVector3Normalize(binormal);

							XMStoreFloat3(&tangent[j][i], tan);
							XMStoreFloat3(&binorm[j][i], binormal);
						}
					}



					// shader version
					/*
					float3 c1 = cross(output.normal, float3(0.0, 0.0, 1.0));
					float3 c2 = cross(output.normal, float3(0.0, 1.0, 0.0));

					if (length(c1)>length(c2))
					{
						tangent = c1;
					}
					else
					{
						tangent = c2;
					}

					output.tangent = normalize(tangent);

					binormal = cross(output.normal, output.tangent);
					output.binormal = normalize(binormal);
					//cols[j][i].x = 10.0f;
					*/
				}

			}


			UpdateBuffers();

			/*
			if (bUpdateable == true)
			{
				UpdateUpdatebleVertexBuffer();
			}
			else
			{
				SetVertexBuffer();
			}
			*/
			
			//SetIndexBufferSimple();
		}
	});

	
}

void Water::UpdateBuffers()
{
	int j, i;

	for (i = 0; i < ycoords; i++)
	{
		for (j = 0; j < xcoords; j++)
		{
			m_vertices[j + (i*xcoords)].pos.y = height_map[j][i];
			m_vertices[j + (i*xcoords)].norm = normals[j][i];
			m_vertices[j + (i*xcoords)].col = cols[j][i];
			m_vertices[j + (i*xcoords)].tan = tangent[j][i];
			m_vertices[j + (i*xcoords)].bin = binorm[j][i];
			//m_vertices[j + (i*xcoords)].blend_2 = cols[j][i].y;
		}
	}


}


bool Water::SetFlat(int xnum, int ynum, float scale, int xplane)
{
	int i, j;
	m_scale = scale;
	xcoords = xnum;
	ycoords = ynum;
	XMMATRIX m_modelMatrix;
	XMVECTOR data;
	XMVECTORF32 floatingVector = { 0.0f, 0.0f, 0.0f, 1.0f };
	data = floatingVector;

	unsigned int seed = 237;
	float pnscale = 0.8f;
	PerlinNoise pn(seed);
	float noise_z = 0.5f;


	no_phy_verticies = ((xnum)*(ynum)) * 8;
	height_fields = new float[xnum*ynum];
	m_phy_vertices = new VertexType[no_phy_verticies];

	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f));

	height_map = new float*[ynum];
	normals = new XMFLOAT3*[ynum];
	cols = new XMFLOAT4*[ynum];
	land_dist = new float*[ynum];

	tangent = new XMFLOAT3*[ynum];
	binorm = new XMFLOAT3*[ynum];

	b_ren = new bool*[ynum];

	for (i = 0; i < ynum; ++i) {
		height_map[i] = new float[xnum];
		normals[i] = new XMFLOAT3[xnum];
		cols[i] = new XMFLOAT4[xnum];
		land_dist[i] = new float[xnum];

		tangent[i] = new XMFLOAT3[xnum];
		binorm[i] = new XMFLOAT3[xnum];

		b_ren[i] = new bool[xnum];
	}

	for (i = 0; i < ynum; i++)
	{
		for (j = 0; j < xnum; j++)
		{
			height_map[j][i] = 0.0f;
			height_map[j][i] = (pn.noise((float)(j+ xblockpos)*pnscale, (float)(i+ yblockpos)*pnscale, noise_z))*1.0f;
			normals[j][i] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			land_dist[j][i] = 0.0f;
			height_fields[j + (i*xnum)] = height_map[i][j];

			b_ren[j][i] = false;
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
	
	//SetIndexBufferSimple();

	return true;


}




void Water::CreateBuffers()
{
	int i, j;
	int end_dist;



	m_vertices.clear();
	m_indices_simple.clear();
	for (j = 0; j<ycoords; j++)
	{
		for (i = 0; i<xcoords; i++)
		{
			AddVertexTexNormCol(
				(float(i)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				height_map[i][j],
				(float(j)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				(2.0f / float(xcoords - 1))*float(i),
				(2.0f / float(ycoords - 1))*float(j),
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

	/*
	// make simple indicies for distant terrain
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
	*/


	//MakeIndices();
	//SetIndexBuffer();



	m_verticesCount = m_vertices.size();


}



void Water::MakeIndices()
{
	int end_dist = 2;
	bool bHoles = false;

	float floor_limit = 2.0f;

	m_indices.clear();
	
	if (true)
	{
		for (int j = 0; j < ycoords - end_dist; j += 2)
		{
			for (int i = 0; i < xcoords - end_dist; i += 2)
			{
				if (land_dist[i][j] > floor_limit)
					b_ren[i][j] = false;
				if (land_dist[i+1][j] > floor_limit)
					b_ren[i+1][j] = false;
				if (land_dist[i][j+1] > floor_limit)
					b_ren[i][j+1] = false;
				if (land_dist[i+1][j+1] > floor_limit)
					b_ren[i+1][j+1] = false;

				if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f))
				{

					if (land_dist[i][j] > floor_limit &&
						land_dist[i + 1][j] > floor_limit &&
						land_dist[i + 1][j + 1] > floor_limit)
					{
						//b_ren[i][j] = false;
						//b_ren[i + 1][j] = false;
						//b_ren[i + 1][j + 1] = false;
					}
					else
					{
						m_indices.push_back(i + (j*xcoords));
						m_indices.push_back((i + 1) + (j*xcoords));
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						b_ren[i][j] = true;
						b_ren[i + 1][j] = true;
						b_ren[i + 1][j + 1] = true;
					}
				}

				if (bHoles == false || !(m_vertices[i + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 1)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i][j] > floor_limit &&
						land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i][j + 1] > floor_limit)
					{
						//b_ren[i][j] = false;
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i][j + 1] = false;
					}
					else
					{
						m_indices.push_back(i + (j*xcoords));
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back(i + ((j + 1)*xcoords));
						b_ren[i][j] = true;
						b_ren[i + 1][j + 1] = true;
						b_ren[i][j + 1] = true;
					}
				}
				if (bHoles == false || !(m_vertices[(i + 1) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i + 1][j] > floor_limit &&
						land_dist[i + 2][j] > floor_limit &&
						land_dist[i + 1][j + 1] > floor_limit)
					{
						//b_ren[i + 1][j] = false;
						//b_ren[i + 2][j] = false;
						//b_ren[i + 1][j + 1] = false;
					}
					else
					{
						m_indices.push_back((i + 1) + (j*xcoords));
						m_indices.push_back((i + 2) + (j*xcoords));
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						b_ren[i + 1][j] = true;
						b_ren[i + 2][j] = true;
						b_ren[i + 1][j + 1] = true;
					}
				}

				if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + (j*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 1)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i + 2][j] > floor_limit &&
						land_dist[i + 2][j + 1] > floor_limit)
					{
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i + 2][j] = false;
						//b_ren[i + 2][j + 1] = false;
					}
					else
					{
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back((i + 2) + (j*xcoords));
						m_indices.push_back((i + 2) + ((j + 1)*xcoords));
						b_ren[i + 1][j + 1] = true;
						b_ren[i + 2][j] = true;
						b_ren[i + 2][j + 1] = true;
					}
				}
				if (bHoles == false || !(m_vertices[i + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i][j + 1] > floor_limit &&
						land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i][j + 2] > floor_limit)
					{
						//b_ren[i][j + 1] = false;
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i][j + 2] = false;
					}
					else
					{
						m_indices.push_back(i + ((j + 1)*xcoords));
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back(i + ((j + 2)*xcoords));
						b_ren[i][j + 1] = true;
						b_ren[i + 1][j + 1] = true;
						b_ren[i][j + 2] = true;
					}
				}

				if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[i + ((j + 2)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i + 1][j + 2] > floor_limit &&
						land_dist[i][j + 2] > floor_limit)
					{
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i + 1][j + 2] = false;
						//b_ren[i][j + 2] = false;
					}
					else
					{
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back((i + 1) + ((j + 2)*xcoords));
						m_indices.push_back(i + ((j + 2)*xcoords));
						b_ren[i + 1][j + 1] = true;
						b_ren[i + 1][j + 2] = true;
						b_ren[i][j + 2] = true;
					}
				}
				if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i + 2][j + 1] > floor_limit &&
						land_dist[i + 2][j + 2] > floor_limit)
					{
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i + 2][j + 1] = false;
						//b_ren[i + 2][j + 2] = false;
					}
					else
					{
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back((i + 2) + ((j + 1)*xcoords));
						m_indices.push_back((i + 2) + ((j + 2)*xcoords));
						b_ren[i + 1][j + 1] = true;
						b_ren[i + 2][j + 1] = true;
						b_ren[i + 2][j + 2] = true;
					}
				}
				if (bHoles == false || !(m_vertices[(i + 1) + ((j + 1)*xcoords)].pos.y == 0.0f && m_vertices[(i + 2) + ((j + 2)*xcoords)].pos.y == 0.0f && m_vertices[(i + 1) + ((j + 2)*xcoords)].pos.y == 0.0f))
				{
					if (land_dist[i + 1][j + 1] > floor_limit &&
						land_dist[i + 2][j + 2] > floor_limit &&
						land_dist[i + 1][j + 2] > floor_limit)
					{
						//b_ren[i + 1][j + 1] = false;
						//b_ren[i + 2][j + 2] = false;
						//b_ren[i + 1][j + 2] = false;
					}
					else
					{
						m_indices.push_back((i + 1) + ((j + 1)*xcoords));
						m_indices.push_back((i + 2) + ((j + 2)*xcoords));
						m_indices.push_back((i + 1) + ((j + 2)*xcoords));
						b_ren[i + 1][j + 1] = true;
						b_ren[i + 2][j + 2] = true;
						b_ren[i + 1][j + 2] = true;
					}
				}
			}
		}
	}


	if (false)
	{
		m_indices.clear();
		for (int j = 0; j < ycoords - end_dist; j += 2)
		{
			for (int i = 0; i < xcoords - end_dist; i += 2)
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
	}
	no_phy_verticies = 0;

	m_indicesCount = m_indices.size();
	m_indicesCount_simple = 0;// m_indices_simple.size();

	SetIndexBuffer();
}

void Water::AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a)
{
	unique_ptr<WaterVertexData> vd(new WaterVertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	vd->SetColor(r, g, b,a);
	vd->SetBlend(0.0f);
	vd->SetBlend2(0.0f);
	//vd->SetBlend(1.0f - ny);

	m_vertices.push_back(*vd);
}

void Water::AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz)
{
	unique_ptr<WaterVertexData> vd(new WaterVertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	//vd->SetColor(DirectX::XMFLOAT4(rand_brightness, rand_brightness, rand_brightness, 1.0f));
	m_vertices.push_back(*vd);
}


void Water::SetVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_vertices.data(), 0, 0 };
	UINT bytes = sizeof(WaterVertexData) * m_verticesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_vertexBuffer));
}


void Water::SetIndexBuffer()
{
	if (m_indicesCount > 0)
	{
		D3D11_SUBRESOURCE_DATA bufferData = { m_indices.data(), 0, 0 };
		UINT bytes = sizeof(unsigned short) * m_indicesCount;
		CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
		ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer));
	}
}

void Water::SetIndexBufferSimple()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices_simple.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount_simple;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer_simple));
}

Water::~Water(void)
{
	ClearMemory();
}


void Water::ClearMemory()
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

