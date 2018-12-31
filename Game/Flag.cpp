#include "pch.h"
#include "Flag.h"
#include "PerlinNoise.h"



//#if defined __ARMCC_VERSION
#if defined _WIN64

#else
#pragma pack(2) // Add this

#if defined _WIN32


#else


typedef struct
{
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
} BITMAPFILEHEADER;

#pragma pack() // and this

#endif
#endif


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;

Flag::Flag(AllResources* p_Resources) :
	m_Resources(p_Resources)
{
	FILE* f_ptr;
	char filename[100];

	lev_num = 1;

	m_deviceResources = m_Resources->m_deviceResources;

	total_planes = 0;

	m_Texture = nullptr;


	fin_state = 0;

	noise_scale = 0.2f;

	bLoadedShadows = false;


	ang = 0.0f;

}



int Flag::clampB(int pX, int pMax)
{
	if (pX > pMax)
	{
		return pMax;
	}
	else if (pX < 0)
	{
		return 0;
	}
	else
	{
		return pX;
	}
}







/// <summary>
/// Get the height of the terrain at given horizontal coordinates.
/// </summary>
/// <param name="xPos">X coordinate</param>
/// <param name="zPos">Z coordinate</param>
/// <returns>Height at given coordinates</returns>
float Flag::GetTerrainHeight(float xPos, float zPos)
{
	float scaleFactor = 2.0f;
	// we first get the height of four points of the quad underneath the point
	// Check to make sure this point is not off the map at all

	xPos += left_right_walls - 1.0f;
	zPos += front_back_walls - 1.0f;

	//xPos =  xPos;
	//zPos =  zPos;
	int x = (int)(xPos / 2.0f);
	int z = (int)(zPos / 2.0f);


	int xPlusOne = x + 1;
	int zPlusOne = z + 1;

	if (x > total_x_points - 2)
		x = total_x_points - 2;
	//return -999.0f;
	if (x < 1)
		x = 1;
	//return -999.0f;
	if (z > total_y_points - 2)
		z = total_y_points - 2;
	//return -999.0f;
	if (z < 0)
		z = 0;


	float triZ0 = height_map[z][x];
	float triZ1 = height_map[z][x + 1];
	float triZ2 = height_map[z + 1][x];
	float triZ3 = height_map[z + 1][x + 1];

	float height = 0.0f;
	float sqX = (xPos / scaleFactor) - (float)x;
	float sqZ = (zPos / scaleFactor) - (float)z;
	if ((sqX + sqZ) < 1)
	{
		height = triZ0;
		height += (triZ1 - triZ0) * sqX;
		height += (triZ2 - triZ0) * sqZ;
	}
	else
	{
		height = triZ3;
		height += (triZ1 - triZ3) * (1.0f - sqZ);
		height += (triZ2 - triZ3) * (1.0f - sqX);
	}
	height = height*0.5f;
	return height*scaleFactor;
}



btVector3 Flag::GetNormal(float xPos, float zPos)
{
	float scaleFactor = 2.0f;

	xPos += left_right_walls - 1.0f;
	zPos += front_back_walls - 1.0f;

	//xPos =  xPos;
	//zPos =  zPos;
	int x = (int)(xPos / 2.0f);
	int z = (int)(zPos / 2.0f);


	int xPlusOne = x + 1;
	int zPlusOne = z + 1;

	if (x > total_x_points - 2)
		x = total_x_points - 2;
	//return -999.0f;
	if (x < 1)
		x = 1;
	//return -999.0f;
	if (z > total_y_points - 2)
		z = total_y_points - 2;
	//return -999.0f;
	if (z < 0)
		z = 0;
	//return -999.0f;


	/*
	if (x > total_x_points - 2)
	return btVector3(-999.0f, -999.0f, -999.0f);
	if (x < 1)
	return btVector3(-999.0f, -999.0f, -999.0f);
	if (z > total_y_points - 2)
	return btVector3(-999.0f, -999.0f, -999.0f);
	if (z < 0)
	return btVector3(-999.0f, -999.0f, -999.0f);
	*/


	btVector3 triZ0 = btVector3(normals[z][x].x, normals[z][x].y, normals[z][x].z);//(m_hminfo.heightMap[x*m_hminfo.terrainWidth + z].y);
	btVector3 triZ1 = btVector3(normals[z][x + 1].x, normals[z][x + 1].y, normals[z][x + 1].z);//(m_hminfo.heightMap[xPlusOne*m_hminfo.terrainWidth + z].y);
	btVector3 triZ2 = btVector3(normals[z + 1][x].x, normals[z + 1][x].y, normals[z + 1][x].z);//(m_hminfo.heightMap[x*m_hminfo.terrainWidth + zPlusOne].y);
	btVector3 triZ3 = btVector3(normals[z + 1][x + 1].x, normals[z + 1][x + 1].y, normals[z + 1][x + 1].z);//(m_hminfo.heightMap[xPlusOne*m_hminfo.terrainWidth + zPlusOne].y);

	btVector3 avgNormal;


	float sqX = (xPos / scaleFactor) - (float)x;
	float sqZ = (zPos / scaleFactor) - (float)z;
	if ((sqX + sqZ) < 1)
	{
		avgNormal = triZ0;
		avgNormal += (triZ1 - triZ0) * sqX;
		avgNormal += (triZ2 - triZ0) * sqZ;
	}
	else
	{
		avgNormal = triZ3;
		avgNormal += (triZ1 - triZ3) * (1.0f - sqZ);
		avgNormal += (triZ2 - triZ3) * (1.0f - sqX);
	}
	return avgNormal;


}


void Flag::Render(float x,float y,float z)
{
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());

	int i;
	current_index = 0;
	for (i = 0; i < total_planes - x_planes; i++)
	{

	}


	XMFLOAT4X4 m_ModelMatrix = m_Resources->m_Camera->m_constantBufferData.model;

	for (i = 0; i<total_planes; i++)
	{
		XMFLOAT3 flag_translation = XMFLOAT3(-7.5f, -0.2f, -1.8f);

		/*
		
		XMFLOAT4X4 m_NodeMatrix;
		m_FlagNode[i]->GetMatrix(&m_NodeMatrix);

		m_NodeMatrix._14 += flag_translation.x;
		m_NodeMatrix._24 += flag_translation.y;
		m_NodeMatrix._34 += flag_translation.z;

		XMMATRIX nodeMatrix = XMLoadFloat4x4(&m_NodeMatrix);

		//XMMATRIX translationMatrix = XMMatrixTranslation(1.0f, 0.0f, 0.0f);
		//nodeMatrix = nodeMatrix * translationMatrix;
		XMFLOAT4X4 new_matrix = m_ModelMatrix;

		XMMATRIX xm_modelMatrix = XMLoadFloat4x4(&m_ModelMatrix);
		
		
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(M_PI*0.5f, M_PI*0.5f+ M_PI, 0.0f);

		xm_modelMatrix = xm_modelMatrix * rotationMatrix;// (translationMatrix * rotationMatrix);

		XMMATRIX modelMatrix = xm_modelMatrix * nodeMatrix;
		*/
		
		XMFLOAT4X4 m_NodeMatrix;
		m_FlagNode[i]->GetMatrix(&m_NodeMatrix);


		XMMATRIX nodeMatrix = XMLoadFloat4x4(&m_NodeMatrix);

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(M_PI*0.5f, -M_PI*0.5f, 0.0f);
		XMMATRIX rotationMatrix2 = XMMatrixRotationRollPitchYaw(m_Resources->m_LevelInfo.flag_angle, 0.0f, 0.0f);

		/*
		ang = ang + 0.0001f;
		if (ang > M_PI*2.0f)
		{
			ang = 0.0f;
		}
		*/

		XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 10.0f, 0.0f);
		XMMATRIX identityMatrix = XMMatrixIdentity();
		XMMATRIX scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);

		XMMATRIX modelMatrix = rotationMatrix * rotationMatrix2 * nodeMatrix;

		//modelMatrix = XMMatrixMultiply(translationMatrix, modelMatrix);

		XMStoreFloat4x4(&m_Resources->m_Camera->m_constantBufferData.model, modelMatrix);

		m_Resources->m_Camera->m_constantBufferData.model._14 += x;
		m_Resources->m_Camera->m_constantBufferData.model._24 += y;
		m_Resources->m_Camera->m_constantBufferData.model._34 += z;


		m_Resources->m_Camera->UpdateConstantBuffer();

		m_FlagNode[i]->Render(0);
	}

}




void Flag::Update(XMMATRIX *viewMatrix, float timeTotal)
{
	for (int i = 0; i<total_planes; i++)
	{
		m_FlagNode[i]->Update(viewMatrix, timeTotal);
	}
}



void Flag::UpdateFlagNodes()
{
	wchar_t texture_filename[40];
	wchar_t texture_filename_2[40];

	wchar_t ground_filename1[30];
	wchar_t ground_filename2[30];


	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_FlagNode[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours);
		}
	}
}



void Flag::CalculateNormals()
{

	// This is where the normals are worked out
	for (int j = 0; j < total_y_points; j++)
	{
		for (int i = 0; i < total_x_points; i++)
		{
			float tl = height_map[clampB(j - 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float t = height_map[clampB(j - 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float tr = height_map[clampB(j - 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float r = height_map[clampB(j, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float br = height_map[clampB(j + 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float b = height_map[clampB(j + 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float bl = height_map[clampB(j + 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float l = height_map[clampB(j, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;

			float dZ = ((tr + (1.5f * r) + br) - (tl + (1.5f * l) + bl));
			float dX = (tl + (1.5f * t) + tr) - (bl + (1.5f * b) + br);
			float dY = 15.5;// / pStrength;

			btVector3 v(dX, dY, dZ);
			v.normalize();
			v = flag_rot_mat * v;

			normals[j][i].x = (float)v.getX();
			normals[j][i].y = (float)v.getY();
			normals[j][i].z = (float)v.getZ();

		}
	}
}

//void Flag::InitializeTerrain( char* terrain_filename,wchar_t* texture_filename, Physics* phys , float height_scale)
void Flag::UpdateGeneratedTerrain(int Flag, float noise_z, DirectX::XMFLOAT4X4 _rot_mat)
{

	unsigned int seed = 237;
	float scale = 0.08f;
	PerlinNoise pn(seed);

	rot_mat = btMatrix3x3(_rot_mat._11,
		_rot_mat._12,
		_rot_mat._13,
		_rot_mat._21,
		_rot_mat._22,
		_rot_mat._23,
		_rot_mat._31,
		_rot_mat._32,
		_rot_mat._33);


	for (int i = 0; i <total_y_points; i++) {


		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = ((total_x_points-j)*0.05f)*(pn.noise((float)j*scale + noise_z, (float)i*scale, noise_z*0.1f))*m_height_scale;//*
																										//pn.noise((float)j*scale*3.0f, (float)i*scale*3.0f, 1.0f))*20.0f;
		}
	}


	CalculateNormals();

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_FlagNode[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours);

		}
	}



}

// Sets up the arrays
void Flag::InitializeTerrainVariables(int x_nodes, int y_nodes, float terrain_scale)
{
	x_planes = x_nodes;
	y_planes = y_nodes;

	local_terrain_scale = terrain_scale;

	total_x_points = (x_planes * 8) + 1;
	total_y_points = (y_planes * 8) + 1;


	for (int i = 0; i < x_planes*y_planes; i++)
	{
		m_FlagNode[i] = new FlagNode(m_Resources, 1);
	}


	colours = new XMFLOAT4*[total_y_points];
	for (int i = 0; i < total_y_points; i++)
	{
		colours[i] = new XMFLOAT4[total_x_points];
	}

	height_map = new float*[total_y_points];
	for (int i = 0; i <total_y_points; i++)
	{
		height_map[i] = new float[total_x_points];

	}

	normals = new norm_t*[total_y_points];

	for (int i = 0; i <total_y_points; i++) {
		normals[i] = new norm_t[total_x_points];
	}


	total_planes = 0;
	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			//m_Terrain[i + (j*x_planes)] = new Terrain();
			m_FlagNode[i + (j*x_planes)]->InitFlagNode(i * 8, j * 8, x_planes, y_planes);

			float shift = 160.0f;

			float posx = -(shift*terrain_scale) + (((float)(i))*(shift*terrain_scale));
			float posz = -(shift*terrain_scale) + (((float)(j))*(shift*terrain_scale));

			posx += ((10.0f + (80.0f*(x_planes - 2)))*terrain_scale); // for 2
			posz += ((-100.0f+ (80.0f*(y_planes - 2)))*terrain_scale); // for 2

			m_FlagNode[i + (j*x_planes)]->SetPosition(posx, 0.0f, posz);

			m_FlagNode[i + (j*x_planes)]->zpos = posz;
			m_FlagNode[i + (j*x_planes)]->xpos = posx;
			m_FlagNode[i + (j*x_planes)]->cam_z = cam_z;
			total_planes++;
		}
	}

}

void Flag::InitializeGeneratedTerrain(int Flag, float height_scale, float noise_z)
{
	wchar_t texture_filename[40];
	wchar_t texture_filename_2[40];

	wchar_t ground_filename1[30];
	wchar_t ground_filename2[30];

	/*
	rot_mat = btMatrix3x3(_rot_mat._11,
		_rot_mat._12,
		_rot_mat._13,
		_rot_mat._21,
		_rot_mat._22,
		_rot_mat._23,
		_rot_mat._31,
		_rot_mat._32,
		_rot_mat._33);
		*/

	XMFLOAT4X4 flag_rotation;

	DirectX::XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(M_PI*0.5f, M_PI*0.5f + M_PI, 0.0f);

	XMStoreFloat4x4(&flag_rotation, rotationMatrix);

	flag_rot_mat = btMatrix3x3(flag_rotation._11,
		flag_rotation._12,
		flag_rotation._13,
		flag_rotation._21,
		flag_rotation._22,
		flag_rotation._23,
		flag_rotation._31,
		flag_rotation._32,
		flag_rotation._33);

	unsigned int seed = 237;
	float scale = 0.08f;
	PerlinNoise pn(seed);

	m_height_scale = height_scale;

	for (int i = 0; i < total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			colours[i][j] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
		}
	}


	for (int i = 0; i <total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = (pn.noise((float)j*scale, (float)i*scale, noise_z))*m_height_scale;//*
		}
	}

	for (int j = 0; j < total_y_points; j++)
	{
		for (int i = 0; i < total_x_points; i++)
		{
			float tl = height_map[clampB(j - 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float t = height_map[clampB(j - 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float tr = height_map[clampB(j - 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float r = height_map[clampB(j, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float br = height_map[clampB(j + 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float b = height_map[clampB(j + 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float bl = height_map[clampB(j + 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float l = height_map[clampB(j, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;

			float dZ = -((tr + (1.5f * r) + br) - (tl + (1.5f * l) + bl));
			float dX = (tl + (1.5f * t) + tr) - (bl + (1.5f * b) + br);
			float dY = 15.5;// / pStrength;

			btVector3 v(dX, dY, dZ);
			v.normalize();
			v = rot_mat * v;

			normals[j][i].x = (float)v.getX();
			normals[j][i].y = (float)v.getY();
			normals[j][i].z = (float)v.getZ();

		}
	}


	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_FlagNode[i + (j*x_planes)]->LoadFromHeightMap(height_map, normals, colours, 9, 9, (20.0f*local_terrain_scale), i);
			m_FlagNode[i + (j*x_planes)]->SetPosition(m_FlagNode[i + (j*x_planes)]->xpos, 0.0f, m_FlagNode[i + (j*x_planes)]->zpos);
		}
	}

}


Flag::~Flag(void)
{
	ClearMemory();
}

void Flag::ClearMemory()
{
	int i;
	//height_map = new float*[total_y_points];
#ifndef _DEBUG
	if (m_Texture != nullptr)
		m_Texture->Release();
#endif


	for (i = 0; i <total_y_points; i++) {
		delete[] height_map[i]; // = new float[total_x_points];
	}
	delete[] height_map;

	//normals = new norm_t*[total_y_points];

	for (i = 0; i <total_y_points; i++) {
		delete[] normals[i];// = new norm_t[total_x_points];
	}
	delete[] normals;


	//normals = new norm_t*[total_y_points];


	//for (i = 0; i <total_y_points; i++) {
	//	delete[] shadows[i];// = new norm_t[total_x_points];
	//}
	//delete[] shadows;

	for (i = 0; i < total_planes; i++)
	{
		m_FlagNode[i]->ClearMemory();
	}


	//delete[] m_hminfo.heightMap;
	//delete[] m_hminfo.normal;
	//delete[] m_hminfo.colour;
	total_planes = 0;



}
