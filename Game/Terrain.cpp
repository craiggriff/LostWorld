#include "pch.h"
#include "Terrain.h"

//#include "DirectXHelper.h"

using namespace Game;
using namespace DirectX;
using namespace DX;



void SimpleMotion::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void SimpleMotion::setWorldTransform(const btTransform &worldTransform)
{

}

SimpleMotion::SimpleMotion()
{

}

SimpleMotion::~SimpleMotion()
{


}

Terrain::Terrain(AllResources* p_Resources, int _bUpdateable):
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

	
}

void Terrain::InitTerrain(int xbp, int ybp)
{
	xblockpos = xbp;
	yblockpos = ybp;

	start_id = 9999;
	end_id = 0;
	bLoaded = false;


	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f));
}




bool Terrain::CreateUpdatebleVertexBuffer()
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
	vertexBufferDesc.ByteWidth = sizeof(GroundVertexData) * m_verticesCount;
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

bool Terrain::UpdateUpdatebleVertexBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_MAPPED_SUBRESOURCE mappedResource2;

	HRESULT result;
	GroundVertexData* verticesPtr;
	unsigned long* indicesPtr;

	// Lock the vertex buffer.
	result = m_deviceResources->GetD3DDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (GroundVertexData*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices.data(), (sizeof(GroundVertexData) * (m_verticesCount)));

	// Unlock the vertex buffer.
	m_deviceResources->GetD3DDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
}


void Terrain::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void Terrain::setWorldTransform(const btTransform &worldTransform)
{
	m_initialTransform = worldTransform;

	m_rigidbody->setWorldTransform(worldTransform);
}



void Terrain::Render(int complexity)
{
	UINT stride = sizeof(GroundVertexData);
	UINT offset = 0;

	m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, GetVertexBuffer().GetAddressOf(), &stride, &offset);
	if (false)//(complexity == 1)
	{
		m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(GetIndexBufferSimple().Get(), DXGI_FORMAT_R16_UINT, 0);
	}
	else
	{
		m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
	}

	m_deviceResources->GetD3DDeviceContext()->DrawIndexed(m_indicesCount, 0, 0);
}

void Terrain::GetMatrix(DirectX::XMFLOAT4X4* matr)
{
	*matr = m_constantBufferData.model;
	
}

void Terrain::RemovePhysics()
{
	//if (bPhysicsObject == true)
	//{
	//m_Resources->m_Physics.m_dynamicsWorld->getCollisionObjectArray().

	//
	//m_Resources->m_Physics.m_dynamicsWorld->removeCollisionObject(m_collisionShape);
		m_Resources->m_Physics.m_dynamicsWorld->removeRigidBody(m_rigidbody);
		delete m_rigidbody->getMotionState();
		delete m_rigidbody;
		delete m_collisionShape;
		//delete m_box;

		//m_rigidbody->release();
	//}
}

void Terrain::MakePhysicsConvexTriangleTerrain()
{
	//item->Initialize(m_d3dDevice, filename , texture_filename,scale,x,y,z,yaw,pitch,roll);
	int ob_id;
	int triangles = 0;

	//m_rigidbod

	ObjInfo info = { 
		XMFLOAT3(10.0f,50.0f,0.0f),
		XMFLOAT3(0.0f,0.0f,0.0f),
		XMFLOAT3(0.0f,1.0f,1.0f),
		0,
		(COL_CARBODY | COL_OBJECTS | COL_WHEEL ),
		(COL_TERRAIN | COL_RAY) };

	m_box = new SimpleMotion();

	btVector3 A;
	btVector3 B;
	btVector3 C;
	btTriangleMesh* data = new btTriangleMesh();

	for (int i = 0; i<no_phy_verticies; i += 3)
	{
		if (false) //(m_phy_vertices[i].y == 0.0f && m_phy_vertices[i + 1].y == 0.0f && m_phy_vertices[i + 2].y == 0.0f)
		{

		}
		else
		{
			A = btVector3(m_phy_vertices[i].x, m_phy_vertices[i].y, m_phy_vertices[i].z);
			B = btVector3(m_phy_vertices[i + 1].x, m_phy_vertices[i + 1].y, m_phy_vertices[i + 1].z);
			C = btVector3(m_phy_vertices[i + 2].x, m_phy_vertices[i + 2].y, m_phy_vertices[i + 2].z);
			data->addTriangle(A, B, C, false);
		}
		triangles++;
	}

	m_box->setWorldTransform(m_initialTransform);
	m_box->m_initialTransform = m_initialTransform;
	//m_box->
	//btDefaultMotionState* lMotionState = new btDefaultMotionState(tr);

	if (true) //triangles>0)
	{
		//btBvhTriangleMeshShape* fallShape=new btBvhTriangleMeshShape(data,true,true);
		m_collisionShape = new btBvhTriangleMeshShape(data, true);

		//auto fallShape = new btCylinderShape(btCylinderShape(btVector3(1.2,1.2,6.0)));
		//auto fallShape = new btSphereShape(btSphereShape(1.5f));
		//auto fallShape = new btTriangleMesh (btSphereShape(1.5f));
		//btMotionState* fallMotionState = (btMotionState*)&m_box;//(cube, btTransform(btQuaternion(0,0,0,1),btVector3(x,y,z)));
		//ObjInfo dum_objInfo;
		//dum_objInfo.group = (COL_CAR | COL_WORLDSTUFF | COL_BOXES);
		//dum_objInfo.mask = (COL_WALLS);
		//btScalar mass = 0.2;
		btVector3 fallInertia(0, 0, 0);
		m_collisionShape->calculateLocalInertia(0.0f, fallInertia);
		m_rigidbody = m_Resources->m_Physics.AddPhysicalObject(m_collisionShape, (btMotionState *)m_box, fallInertia, &info);
		m_rigidbody->setFriction(0.8f);

		/*pwd
		if (ob_id<start_id)
			ob_id = start_id;

		if (ob_id>end_id)
			ob_id = end_id;
			*/
		bPhysicsObject = true;
	}
	else
	{
		m_rigidbody = nullptr;
	}
}




void Terrain::UpdateProjectionMatrix(XMMATRIX *projectionMatrix)
{
	//XMStoreFloat4x4(&m_constantBufferData.projection, *projectionMatrix);
}

void Terrain::Update(XMMATRIX *viewMatrix, float timeTotal)
{
	XMStoreFloat4x4(&m_constantBufferData.view, *viewMatrix);
}


void Terrain::SetPosition(float x, float y, float z)
{
	XMMATRIX m_modelMatrix;

	pos_x = x;
	pos_y = y;
	pos_z = z;

	auto rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	auto translationMatrix = XMMatrixTranslation(x, y, z);
	auto scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	m_modelMatrix = XMMatrixTranspose((translationMatrix *  rotationMatrix)*scalingMatrix);

	XMStoreFloat4x4(&m_constantBufferData.model, m_modelMatrix);

	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(x, y, z));

}

bool Terrain::UpdateFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols, float** _steepness, float*** _tex_blend)
{
	int i, j,k;

	for (i = 0; i < ycoords; i++)
	{
		for (j = 0; j < xcoords; j++)
		{
			height_map[j][i] = whole_height_map[j + xblockpos][i + yblockpos];
			normals[j][i] = XMFLOAT3(whole_normals[j + xblockpos][i + yblockpos].x, whole_normals[j + xblockpos][i + yblockpos].y, whole_normals[j + xblockpos][i + yblockpos].z);// hm_info->normal[((j)+xblockpos + (((i)+yblockpos)*hm_info->terrainWidth))];

			cols[j][i] = p_cols[j + xblockpos][i + yblockpos];

			steepness[j][i] = _steepness[j + xblockpos][i + yblockpos];
			//tex_blend[j][i] = _tex_blend[j + xblockpos][i + yblockpos];

			height_fields[j + (i*xcoords)] = height_map[i][j];
		}
	}


	for (k = 0; k < 6; k++)
	{
		for (i = 0; i < ycoords; i++)
		{
			for (j = 0; j < xcoords; j++)
			{
				tex_blend[k][j][i] = _tex_blend[k][j + xblockpos][i + yblockpos];
			}
		}
	}



	CreateBuffers();

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


bool Terrain::LoadFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols,int xnum, int ynum, float scale, int xplane, float** _steepness, float*** _tex_blend)
{
	int i, j, k;
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


	tex_blend = new float**[6];

	for (j = 0; j < 6; ++j)
	{
		tex_blend[j] = new float*[ynum];
		for (i = 0; i < ynum; ++i)
		{
			tex_blend[j][i] = new float[xnum];
		}
	}



	height_map = new float*[ynum];
	steepness = new float*[ynum];

	normals = new XMFLOAT3*[ynum];
	cols = new XMFLOAT4*[ynum];

	tangent = new XMFLOAT3*[ynum];
	binorm = new XMFLOAT3*[ynum];


	for (i = 0; i < ynum; ++i) {

		steepness[i] = new float[xnum];
		height_map[i] = new float[xnum];
		normals[i] = new XMFLOAT3[xnum];
		cols[i] = new XMFLOAT4[xnum];
		tangent[i] = new XMFLOAT3[xnum];
		binorm[i] = new XMFLOAT3[xnum];
	}

	for (i = 0; i < ynum; i++)
	{
		for (j = 0; j < xnum; j++)
		{
			height_map[j][i] = whole_height_map[j + xblockpos][i + yblockpos];
			normals[j][i] = XMFLOAT3(whole_normals[j + xblockpos][i + yblockpos].x, whole_normals[j + xblockpos][i + yblockpos].y, whole_normals[j + xblockpos][i + yblockpos].z);// hm_info->normal[((j)+xblockpos + (((i)+yblockpos)*hm_info->terrainWidth))];
			cols[j][i] = p_cols[j + xblockpos][i + yblockpos];
			
			steepness[j][i] = _steepness[j + xblockpos][i + yblockpos];
			//tex_blend[j][i] = _tex_blend[j + xblockpos][i + yblockpos];

			height_fields[j + (i*xnum)] = height_map[i][j];
		}
	}

	for (k = 0; k < 6; k++)
	{
		for (i = 0; i < ynum; i++)
		{
			for (j = 0; j < xnum; j++)
			{
				tex_blend[k][j][i] = _tex_blend[k][j + xblockpos][i + yblockpos];
			}
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
	SetIndexBufferSimple();

	
	return true;

}

void Terrain::CreateBuffers()
{
	int i, j;
	int end_dist;


	m_indices.clear();
	m_vertices.clear();
	m_indices_simple.clear();
	for (j = 0; j<ycoords; j++)
	{
		for (i = 0; i<xcoords; i++)
		{
			float coordu = (2.0f / float(xcoords - 1))*float(i);
			float coordv = (2.0f / float(ycoords - 1))*float(j);

			AddVertexTexNormCol(
				(float(i)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale), 
				height_map[i][j], 
				(float(j)*m_scale) - ((((float)xcoords - 2)*0.5f)*m_scale),
				coordu,
				coordv,
				normals[i][j].x, 
				normals[i][j].y, 
				normals[i][j].z, 
				cols[i][j].x, 
				cols[i][j].y, 
				cols[i][j].z, 
				cols[i][j].w,
				steepness[i][j],
				tex_blend[0][i][j],
				tex_blend[1][i][j],
				tex_blend[2][i][j],
				tex_blend[3][i][j],
				tex_blend[4][i][j],
				tex_blend[5][i][j]
			
				);
		}
	}
	end_dist = 2;

	CalculateCenterHeight();

	int cur_phy = 0;

	bool bHoles = false;

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


	if (false)
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

	CalculateTerrainVectors(); // calculate tangents & binormals
}

void Terrain::CalculateTerrainVectors()
{
	int faceCount, i, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;


	// Calculate the number of faces in the terrain model.
	faceCount = m_indicesCount / 3;

	// Initialize the index to the model data.
	index = 0;
	
	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (i = 0; i<faceCount; i++)
	{
		// Get the three vertices for this face from the terrain model.
		vertex1.x = m_vertices[m_indices[index]].pos.x;
		vertex1.y = m_vertices[m_indices[index]].pos.y;
		vertex1.z = m_vertices[m_indices[index]].pos.z;
		vertex1.tu = m_vertices[m_indices[index]].tex.x;
		vertex1.tv = m_vertices[m_indices[index]].tex.y;
		vertex1.nx = m_vertices[m_indices[index]].norm.x;
		vertex1.ny = m_vertices[m_indices[index]].norm.y;
		vertex1.nz = m_vertices[m_indices[index]].norm.z;
		index++;

		vertex2.x = m_vertices[m_indices[index]].pos.x;
		vertex2.y = m_vertices[m_indices[index]].pos.y;
		vertex2.z = m_vertices[m_indices[index]].pos.z;
		vertex2.tu = m_vertices[m_indices[index]].tex.x;
		vertex2.tv = m_vertices[m_indices[index]].tex.y;
		vertex2.nx = m_vertices[m_indices[index]].norm.x;
		vertex2.ny = m_vertices[m_indices[index]].norm.y;
		vertex2.nz = m_vertices[m_indices[index]].norm.z;
		index++;

		vertex3.x = m_vertices[m_indices[index]].pos.x;
		vertex3.y = m_vertices[m_indices[index]].pos.y;
		vertex3.z = m_vertices[m_indices[index]].pos.z;
		vertex3.tu = m_vertices[m_indices[index]].tex.x;
		vertex3.tv = m_vertices[m_indices[index]].tex.y;
		vertex3.nx = m_vertices[m_indices[index]].norm.x;
		vertex3.ny = m_vertices[m_indices[index]].norm.y;
		vertex3.nz = m_vertices[m_indices[index]].norm.z;

		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the tangent and binormal for this face back in the model structure.
		m_vertices[m_indices[index-1]].tan.x = tangent.x;
		m_vertices[m_indices[index - 1]].tan.y = tangent.y;
		m_vertices[m_indices[index - 1]].tan.z = tangent.z;
		m_vertices[m_indices[index - 1]].bin.x = -binormal.x;
		m_vertices[m_indices[index - 1]].bin.y = -binormal.y;
		m_vertices[m_indices[index - 1]].bin.z = -binormal.z;

		m_vertices[m_indices[index - 2]].tan.x = tangent.x;
		m_vertices[m_indices[index - 2]].tan.y = tangent.y;
		m_vertices[m_indices[index - 2]].tan.z = tangent.z;
		m_vertices[m_indices[index - 2]].bin.x = -binormal.x;
		m_vertices[m_indices[index - 2]].bin.y = -binormal.y;
		m_vertices[m_indices[index - 2]].bin.z = -binormal.z;

		m_vertices[m_indices[index - 3]].tan.x = tangent.x;
		m_vertices[m_indices[index - 3]].tan.y = tangent.y;
		m_vertices[m_indices[index - 3]].tan.z = tangent.z;
		m_vertices[m_indices[index - 3]].bin.x = -binormal.x;
		m_vertices[m_indices[index - 3]].bin.y = -binormal.y;
		m_vertices[m_indices[index - 3]].bin.z = -binormal.z;
	}

	return;
}






void Terrain::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of the tangent.
	length = (float)sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the tangent and then store it.
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of the binormal.
	length = (float)sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the binormal and then store it.
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}

void Terrain::CalculateCenterHeight()
{
	int i, j;
	float count=0;
	float height = 0.0f;

	average_height = 0.0f;
	for (i = 0; i < m_vertices.size(); i++)
	{
		average_height += m_vertices.at(i).pos.y;
		average_height *= 0.5f;
	}


	//average_height = height / float(count);


	furthest_point = 0.0f;
	float len;
	btVector3 v;
	for (i = 0; i < m_vertices.size(); i++)
	{
		v.setX(m_vertices.at(i).pos.x);
		v.setZ(m_vertices.at(i).pos.z);
		v.setY(m_vertices.at(i).pos.y - average_height);

		len = v.length();

		if (len > furthest_point)
			furthest_point = len;
	}


}


void Terrain::AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a,float b1, float bl1, float bl2, float bl3, float bl4, float bl5, float bl6)
{
	unique_ptr<GroundVertexData> vd(new GroundVertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	vd->SetColor(r, g, b, a);


	vd->SetBlend(b1);
	vd->SetBlend2(bl1);
	vd->SetBlend3(bl2);
	vd->SetBlend4(bl3);
	vd->SetBlend5(bl4);
	vd->SetBlend6(bl5);
	vd->SetBlend7(bl6);
	//vd->SetBlend(1.0f - ny);

	// work out binormal and tangent
	if (false)
	{
		XMVECTOR tan;

		XMVECTOR c1 = XMVector3Cross(XMLoadFloat3(&XMFLOAT3(nx, ny, nz)), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f)));
		XMVECTOR c2 = XMVector3Cross(XMLoadFloat3(&XMFLOAT3(nx, ny, nz)), XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)));

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

		XMVECTOR binormal = XMVector3Cross(XMLoadFloat3(&XMFLOAT3(nx, ny, nz)), tan);
		binormal = XMVector3Normalize(binormal);

		XMStoreFloat3(&vd->tan, tan);
		XMStoreFloat3(&vd->bin, binormal);
	}

	m_vertices.push_back(*vd);
}




void Terrain::AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz)
{
	unique_ptr<GroundVertexData> vd(new GroundVertexData());
	//float rand_brightness = float(rand() % 100)*0.01f;
	vd->SetPosition(x, y, z);
	vd->SetTex(u, v);
	vd->SetNormal(nx, ny, nz);

	//vd->SetColor(DirectX::XMFLOAT4(rand_brightness, rand_brightness, rand_brightness, 1.0f));
	m_vertices.push_back(*vd);
}


void Terrain::SetVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_vertices.data(), 0, 0 };
	UINT bytes = sizeof(GroundVertexData) * m_verticesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_vertexBuffer));
}


void Terrain::SetIndexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer));
}

void Terrain::SetIndexBufferSimple()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices_simple.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount_simple;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer_simple));
}

Terrain::~Terrain(void)
{
	ClearMemory();
}


void Terrain::ClearMemory()
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
