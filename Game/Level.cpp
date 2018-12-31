#include "pch.h"
#include "Level.h"
#include "PerlinNoise.h"




//#if defined __ARMCC_VERSION
#if defined _WIN64

#else

#if defined _WIN32

#else

#pragma pack(2) // Add this

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

// std::sort with inlining
struct terrain_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

float Level::flerp(float a, float b, float f)
{
	return (a * (1.0f - f)) + (b * f);
}


Level::Level(AllResources* p_Resources,Sky* p_Sky):
	m_Resources(p_Resources),
	m_Sky(p_Sky)
{
	FILE* f_ptr;
	char filename[100];
	wchar_t texture_filename[40];
	wchar_t texture_filename_2[40];
	wchar_t texture_filename_3[40];

	lev_num = 1;

	m_deviceResources = m_Resources->m_deviceResources;

	total_planes = 0;


	fin_state = 0;

	current_index = 0;

	noise_scale = 0.2f;

	bLoadedShadows = false;

	p_index.clear();


	m_Water_Texture = m_Resources->m_Textures->LoadTexture("water2");
	m_Water_Texture_2 = m_Resources->m_Textures->LoadTexture("water2");
	m_Water_Texture_3 = m_Resources->m_Textures->LoadTexture("yell");

	m_Water_Norm = m_Resources->m_Textures->LoadTexture("trees_bark_002_nor"); 
	//m_Water_Norm = m_Resources->m_Textures->LoadTexture("waternorm_1"); trees_bark_002_nor
	m_Water_Norm2 = m_Resources->m_Textures->LoadTexture("waternorm_2");

	m_Paths = new TerrainPaths(m_Resources);

}



int Level::clampB(int pX, int pMax)
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






bool Level::LoadBinary(int level)
{
	int i,j;

	char info_filename[140];

	if (m_Resources->bContentFolder == false)
	{
		sprintf_s(info_filename, "%s\\LevelBinary\\%d.bmp", m_Resources->local_file_folder, level);
	}
	else
	{
		sprintf_s(info_filename, "Assets\\LevelBinary\\%d.bmp", level);
	}

	FILE * pFile;

	fopen_s(&pFile,info_filename, "rb");
	if (pFile != NULL)
	{
		for (i = 0; i < total_y_points; i++)
		{
			fread(height_map[i], sizeof(float), total_x_points, pFile);
		}
		for (i = 0; i < total_y_points; i++)
		{
			fread(normals[i], sizeof(norm_t), total_x_points, pFile);
		}
		for (i = 0; i < total_y_points; i++)
		{
			fread(colours[i], sizeof(XMFLOAT4), total_x_points, pFile);
		}

		for (i = 0; i < total_y_points; i++)
		{
			for (j = 0; j < total_x_points; j++)
			{
				height_map_to[i][j] = height_map[i][j];
			}
		}


		for (j = 0; j < 6; j++)
		{
			for (i = 0; i < total_y_points; i++)
			{
				fread(tex_blend[j][i], sizeof(float), total_x_points, pFile);
			}
		}


		fclose(pFile);

		

		CreateNormals();
		SetupTerrainColours();
		UpdateTerrain(true);
		LoadTextures();
		SetWaterActive();
		return true;
	}
	else
	{
		UpdateTerrain(true);
		return false;
	}
}

bool Level::SaveBinary(int level)
{
	int i,j;
	char info_filename[140];

	sprintf_s(info_filename, "%s\\LevelBinary\\%d.bmp", m_Resources->local_file_folder, level);

	FILE * pFile;

	fopen_s(&pFile,info_filename, "wb");
	if (pFile != NULL)
	{
		for (i = 0; i < total_y_points; i++)
		{
			fwrite(height_map[i], sizeof(float), total_x_points, pFile);
		}
		for (i = 0; i < total_y_points; i++)
		{
			fwrite(normals[i], sizeof(norm_t), total_x_points, pFile);
		}
		for (i = 0; i < total_y_points; i++)
		{
			fwrite(colours[i], sizeof(XMFLOAT4), total_x_points, pFile);
		}


		for (j = 0; j < 6; j++)
		{
			for (i = 0; i < total_y_points; i++)
			{
				fwrite(tex_blend[j][i], sizeof(float), total_x_points, pFile);
			}
		}



		fclose(pFile);

	}
	else
	{
		return false;
	}

	return true;
}

float Level::GetWaterHeight(float zPos, float xPos)
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


	float triZ0 = wave_height_map[z][x];
	float triZ1 = wave_height_map[z][x + 1];
	float triZ2 = wave_height_map[z + 1][x];
	float triZ3 = wave_height_map[z + 1][x + 1];

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

/// <summary>
/// Get the height of the terrain at given horizontal coordinates.
/// </summary>
/// <param name="xPos">X coordinate</param>
/// <param name="zPos">Z coordinate</param>
/// <returns>Height at given coordinates</returns>
float Level::GetTerrainHeight(float zPos, float xPos)
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



btVector3 Level::GetNormal(float zPos, float xPos)
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


void Level::SetupTerrainColours()
{
	unsigned int seed = 237;
	float scale = 0.8f;
	PerlinNoise pn(seed);
	//return;
	XMVECTOR dir = XMLoadFloat3(&m_Resources->m_LevelInfo.diff_dir);
	XMVECTOR dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);

	for (int j = 0; j < total_y_points; j++)
	{
		for (int i = 0; i < total_x_points; i++)
		{
			/**/
			float w = colours[j][i].w;
			colours[j][i] = m_Resources->m_LevelInfo.ambient_col;

			XMVECTOR col = XMLoadFloat4(&colours[j][i]);
			XMVECTOR nor = XMLoadFloat3(&XMFLOAT3(normals[j][i].x, 1.0f-steepness[j][i], normals[j][i].z));

			XMVECTOR lightIntensityA = XMVectorSaturate(XMVector3Dot(nor,-dir));
			float lightIntensity;
			XMStoreFloat(&lightIntensity, lightIntensityA);

			if (lightIntensity > 0.0f)
			{
				col = col + XMVectorSaturate(dif*lightIntensity);
				XMStoreFloat4(&colours[j][i], col);
			}

			colours[j][i].w = 0.0f;
	
			for (int k = 0; k < m_Resources->m_Lights->m_pointLights.size(); k++)
			{
				if (m_Resources->m_Lights->m_pointLights[k].spot == 0.0f)
				{
					XMVECTOR l = (XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[k].pos) - XMLoadFloat3(&XMFLOAT3((float)j*2.0f - left_right_walls + 1.0f, height_map[j][i], (float)i*2.0f - front_back_walls + 1.0f)));// / m_Resources->m_Lights->m_pointLights[k].radius;
					float len;
					XMStoreFloat(&len, XMVector3Length(l));
					if (len < m_Resources->m_Lights->m_pointLights[k].radius)
					{
						
						float dot;
						XMStoreFloat(&dot, XMVector3Dot(l, l));
						float atten = 1.0f-(len / m_Resources->m_Lights->m_pointLights[k].radius);//  1.0f - dot;


						l = XMVector3Normalize(l);
						



						XMVECTOR ldif = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[k].diffuse);
						XMVECTOR lamb = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[k].ambient);
						XMVECTOR ldir = XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[k].dir);

						lightIntensityA = XMVectorSaturate(XMVector3Dot(nor, l));

						XMVECTOR out = ((ldif*lightIntensityA)+ lamb)*atten;
						XMFLOAT4 light_col;
						XMStoreFloat4(&light_col, out);
						
						colours[j][i].x += light_col.x;
						colours[j][i].y += light_col.y;
						colours[j][i].z += light_col.z;
						//colours[j][i].x /= 1.0f - light_col.x;
						//colours[j][i].y /= 1.0f - light_col.y;
						//colours[j][i].z /= 1.0f - light_col.z;
						//col.w += light_col.w;
					}
				}
			}

			colours[j][i].w = w;

				//normals[j][i].
			/*
			colours[j][i].x = (((pn.noise((float)j*scale*0.5f, (float)i*scale*0.5f, 1.0f)))*0.3f) + 0.5f;
			colours[j][i].y = ((pn.noise((float)j*scale, (float)i*scale, 1.1f))*0.2f);
			colours[j][i].z = ((pn.noise((float)j*scale, (float)i*scale, 1.2f))*0.2f) - 0.1f;

			colours[j][i].x += (((pn.noise((float)j*scale*0.2f, (float)i*scale*0.2f, 1.0f)))*0.3f) + 0.5f;
			colours[j][i].y += ((pn.noise((float)j*scale*2.0f, (float)i*scale*2.0f, 1.1f))*0.2f) - 0.1f;
			colours[j][i].z += ((pn.noise((float)j*scale*2.0f, (float)i*scale*2.0f, 1.2f))*0.2f) - 0.1f;
			*/
			//colours[j][i].x = (((pn.noise((float)j*scale*0.5f, (float)i*scale*0.5f, 1.0f)))*0.8f) + 0.5f;

			//colours[j][i].z = normals[j][i].y;
		}

	}

}

void Level::RenderWater()
{
	int i;

	mat_buffer.normal_height = 0.0f;
	mat_buffer.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat_buffer.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat_buffer.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat_buffer.normal_height = 1.0f;
	m_Resources->UpdateMaterialBuffer(&mat_buffer);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Water_Texture.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_Water_Texture_2.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(2, 1, m_Water_Texture_3.GetAddressOf());

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(3, 1, m_Water_Norm.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(4, 1, m_Water_Norm2.GetAddressOf());


	for (i = 0; i < total_planes; i++)
	{
		m_Water[i]->GetMatrix(m_Resources->ConstantModelBuffer());
		m_Resources->m_Camera->UpdateConstantBuffer();

		if (m_Water[i]->bActive == true && m_Water[i]->cam_check_point > 0.0f)
		{
			m_Water[i]->UpdateUpdatebleVertexBuffer();
			m_Water[i]->Render(0);
		}
	}
}

void Level::Render()
{
	int i;

	// ground material
	// Update the material constant buffer.
	mat_buffer.normal_height = 1.0f;
	mat_buffer.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat_buffer.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat_buffer.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);

	m_Resources->UpdateMaterialBuffer(&mat_buffer);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(20, 8, m_TextureArray);
	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(30, 8, m_NormalArray);


	

	for (i = 0; i < current_index; i++)
	{
		m_Terrain[p_index[i].part_no]->GetMatrix(m_Resources->ConstantModelBuffer());
		m_Resources->m_Camera->UpdateConstantBuffer();

		m_Terrain[p_index[i].part_no]->Render(0);

	}


	/*
	for (i = 0; i<total_planes; i++)
	{
		m_Terrain[i]->GetMatrix(m_Resources->ConstantModelBuffer());
		m_Resources->m_Camera->UpdateConstantBuffer();

		if (m_Terrain[i]->cam_check_point > 0.0f)
		{
			m_Terrain[i]->Render(0);
			//m_Terrain[i]->m_rigidbody->setActivationState(1);
		}
	
	}
	*/
	/*
	for (i = total_planes - x_planes; i<total_planes; i++)
	{
		if (true)//(m_camera->CheckPoint(m_Terrain[i]->xpos, 1.0f, m_Terrain[i]->zpos, 12.0f, FAR_DIST + 80.0f))
		{
			m_Terrain[i]->Render(0);
			m_Terrain[i]->m_rigidbody->setActivationState(1);
		}
		else
		{
			m_Terrain[i]->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
			m_Terrain[i]->m_rigidbody->setActivationState(0);
		}
	}
	*/
	//deviceContext->PSSetShaderResources(1, 1, nullptr);
}




void Level::Update(XMMATRIX *viewMatrix, float timeTotal)
{
	for (int i = 0; i<total_planes; i++)
	{
		m_Terrain[i]->Update(viewMatrix, timeTotal);
	}
}

void Level::SetFriction(float fric)
{
	int i, j;

	for (j = 0; j < y_planes; j++)
	{
		for (i = 0; i < x_planes; i++)
		{
			m_Terrain[i + (j*x_planes)]->m_rigidbody->setFriction(fric);
		}
	}

	m_GroundPlane->setFriction(fric);
	m_FloorPlane->setFriction(fric);
}

void Level::SetRestitution(float rest)
{
	int i, j;

	for (j = 0; j < y_planes; j++)
	{
		for (i = 0; i < x_planes; i++)
		{
			m_Terrain[i + (j*x_planes)]->m_rigidbody->setRestitution(rest);
		}
	}
	m_GroundPlane->setRestitution(rest);
	m_FloorPlane->setRestitution(rest);
}

inline bool exists_test1(char* name) 
{
	FILE *file;
	fopen_s(&file, name, "r");
	if (file) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

void Level::LoadTextures()
{
#ifndef _DEBUG
	/*
	if (m_Texture != nullptr)
		m_Texture->Release();

	if (m_Texture_2 != nullptr)
		m_Texture_2->Release();

	if (m_Texture_3 != nullptr)
		m_Texture_3->Release();
		*/
#endif

	m_TextureArray[0] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex1).Get();
	m_TextureArray[1] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex2).Get();
	m_TextureArray[2] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex3).Get();
	m_TextureArray[3] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex4).Get();
	m_TextureArray[4] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex5).Get();
	m_TextureArray[5] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex6).Get();
	m_TextureArray[6] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex7).Get();
	m_TextureArray[7] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_tex8).Get();

	m_NormalArray[0] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal1).Get();
	m_NormalArray[1] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal2).Get();
	m_NormalArray[2] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal3).Get();
	m_NormalArray[3] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal4).Get();
	m_NormalArray[4] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal5).Get();
	m_NormalArray[5] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal6).Get();
	m_NormalArray[6] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal7).Get();
	m_NormalArray[7] = m_Resources->m_Textures->LoadTexture(m_Resources->m_LevelInfo.terrain_normal8).Get();


}

void Level::UpdateTerrain(bool _update_all)
{
	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			if (bPlaneUpdate[i][j] == true || _update_all==true)
			{
				m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours,steepness, tex_blend);
				m_Water[i + (j*x_planes)]->LoadTerrainHeight(height_map);
				if (_update_all == false)
				{
					m_Terrain[i + (j*x_planes)]->RemovePhysics();
				}
				m_Terrain[i + (j*x_planes)]->MakePhysicsConvexTriangleTerrain();
			}
			bPlaneUpdate[i][j] = false;
		}
	}
}

void Level::BakePinch()
{
	for (int i = 0; i < total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = height_map_to[i][j];
		}
	}
	CreateNormals();
	SetupTerrainColours();
	UpdateTerrain();
	SetWaterActive();
}


void Level::ClearPlaneUpdate()
{
	for (int i = 0; i <y_planes; i++) {
		for (int j = 0; j<x_planes; j++)
		{
			bPlaneUpdate[i][j] = false;
		}
	}

	for (int i = 0; i < total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{
			height_map_to[i][j] = height_map[i][j];
		}
	}


}

void Level::UpdatePlaneUpdate(bool bAll)
{
	
	for (int i = 0; i < total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = height_map_to[i][j];
		}
	}
	
	CreateNormals();
	SetupTerrainColours();

	
	

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			if (bPlaneUpdate[i][j] == true || bAll==true)
			{
				//m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours, steepness, m_Paths->tex_blend);

				m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours, steepness, tex_blend);
				m_Water[i + (j*x_planes)]->LoadTerrainHeight(height_map);
				if (bAll == false)
				{
					m_Terrain[i + (j*x_planes)]->RemovePhysics();
				}
				m_Terrain[i + (j*x_planes)]->MakePhysicsConvexTriangleTerrain();
			}

		}
	}
	SetWaterActive();
}

void Level::UpdateGeneratedTerrainPath(float xPos, float yPos, float zPos, float width, int tex_num, bool _dip_path)
{
	xPos += left_right_walls - 1.0f;
	zPos += front_back_walls - 1.0f;
	int x = (int)(xPos / 2.0f);
	int z = (int)(zPos / 2.0f);
	int xPlusOne = x + 1;
	int zPlusOne = z + 1;
	if (x > total_x_points - 2)
		x = total_x_points - 2;
	if (x < 1)
		x = 1;
	if (z > total_y_points - 2)
		z = total_y_points - 2;
	if (z < 0)
		z = 0;




	btVector3 v_pinch = btVector3(z, 0.0f, x);

	for (int i = 0; i <total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{

			btVector3 v_location = btVector3(i, 0.0f, j);
			float dist = v_pinch.distance(v_location);
			if (dist < width)
			{
				if (tex_num == 0)
				{
					float fade = ((cos((dist / width)*3.141f) + 1.0f)*0.1f);
					//float new_col = colours[i][j].w + ((fade - colours[i][j].w)*fade);
					if (true)//(new_col < colours[i][j].w)
					{
						if (m_Paths->tex_blend[i][j]>fade)
							m_Paths->tex_blend[i][j] = fade;

						if (m_Paths->tex_blend[i][j] < 0.0f)
							m_Paths->tex_blend[i][j] = 0.0f;

						
					}
				}
				else
				{
					float fade = ((cos((dist / width)*3.141f) + 1.0f)*0.5f);
					//float new_col = colours[i][j].w + ((fade - colours[i][j].w)*fade);
					if (true)//(new_col > colours[i][j].w)
					{
						if(m_Paths->tex_blend[i][j]<fade)
							m_Paths->tex_blend[i][j] = fade;

						if (m_Paths->tex_blend[i][j] > 1.0f)
							m_Paths->tex_blend[i][j] = 1.0f;
					}
					if (_dip_path == true)
					{
						float h_to = flerp(height_map_to[i][j], yPos, fade);

						height_map_to[i][j] = flerp(height_map_to[i][j],h_to, fade);

						//if(h_to>height_map[i][j])
							
					}
				}

				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;
			}
		}
	}

	



	//BakePinch();

}


void Level::ClearBlendEdit(int _blendnum)
{
	for (int i = 0; i < total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{
			if (_blendnum == 2)
			{
				if (height_map[i][j] < 2.0f)
				{
					float lev = (height_map[i][j]-2.0f)*0.25f;
					if (lev < -1.0f)
						lev = -1.0f;
					tex_blend[_blendnum][i][j] = -lev;
				}
				else
				{
					tex_blend[_blendnum][i][j] = 0.0f;
				}
			}
			else
			{
				tex_blend[_blendnum][i][j] = 0.0f;
			}
		}
	}


}
void Level::UpdateGeneratedTerrainTex(float xPos, float zPos, float width, int tex_num)
{
	xPos += left_right_walls - 1.0f;
	zPos += front_back_walls - 1.0f;
	int x = (int)(xPos / 2.0f);
	int z = (int)(zPos / 2.0f);
	int xPlusOne = x + 1;
	int zPlusOne = z + 1;
	if (x > total_x_points - 2)
		x = total_x_points - 2;
	if (x < 1)
		x = 1;
	if (z > total_y_points - 2)
		z = total_y_points - 2;
	if (z < 0)
		z = 0;


	for (int i = 0; i <y_planes; i++) {
		for (int j = 0; j<x_planes; j++)
		{
			bPlaneUpdate[i][j] = false;
		}
	}

	btVector3 v_pinch = btVector3(z, 0.0f, x);

	for (int i = 0; i <total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{
			height_map_to[i][j] = height_map[i][j];

			btVector3 v_location = btVector3(i, 0.0f, j);
			float dist = v_pinch.distance(v_location);
			if (dist < width)
			{
				if (tex_num == 0)
				{
					float fade = ((cos((dist / width)*3.141f) + 1.0f)*0.5f);
					//float new_col = colours[i][j].w + ((fade - colours[i][j].w)*fade);
					if (true)//(new_col < colours[i][j].w)
					{
						if (m_Paths->tex_blend[i][j]>(1.0f-fade))
							m_Paths->tex_blend[i][j] = (1.0f-fade);

						if (m_Paths->tex_blend[i][j] < 0.0f)
							m_Paths->tex_blend[i][j] = 0.0f;
					}
				}
				else
				{
					float fade = ((cos((dist / width)*3.141f) + 1.0f)*0.5f);
					//float new_col = colours[i][j].w + ((fade - colours[i][j].w)*fade);
					if (true)//(new_col > colours[i][j].w)
					{
						if (m_Paths->tex_blend[i][j]<fade)
							m_Paths->tex_blend[i][j] = fade;

						if (m_Paths->tex_blend[i][j] > 1.0f)
							m_Paths->tex_blend[i][j] = 1.0f;
					}
				}
			
				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;
			}
		}
	}

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			if (bPlaneUpdate[i][j] == true)
			{
				m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map_to, normals, colours, steepness, tex_blend);
			}

		}
	}

}

void Level::UpdateGeneratedTerrainPinch(int level, float noise_z, float xPos, float zPos, float height,float width,bool fixed_height)
{

	unsigned int seed = 237;
	float scale = 0.05f;
	PerlinNoise pn(seed);

	xPos += left_right_walls - 1.0f;
	zPos += front_back_walls - 1.0f;
	int x = (int)(xPos / 2.0f);
	int z = (int)(zPos / 2.0f);
	int xPlusOne = x + 1;
	int zPlusOne = z + 1;
	if (x > total_x_points - 2)
		x = total_x_points - 2;
	if (x < 1)
		x = 1;
	if (z > total_y_points - 2)
		z = total_y_points - 2;
	if (z < 0)
		z = 0;

	btVector3 v_pinch = btVector3(z, 0.0f, x);

	for (int i = 0; i <total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{
			//height_map_to[i][j] = height_map[i][j];

			btVector3 v_location = btVector3(i, 0.0f, j);
			float dist = v_pinch.distance(v_location);
			if (dist < width)
			{
				if (fixed_height == true)
				{
					height_map_to[i][j] = height_map[i][j] + ((cos((dist / width)*3.141f) + 1.0f)*0.5f)*((height) - (height_map[i][j]));
				}
				else
				{
					height_map_to[i][j] = height_map[i][j] + ((cos((dist / width)*3.141f) + 1.0f)*0.5f)*((height_map[z][x] + height) - (height_map[i][j]));
				}
				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j + 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i + 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;
				bPlaneUpdate[clampB((i - 1) / 8, x_planes)][clampB((j - 1) / 8, y_planes)] = true;

			}
			/*
			if (i == z && j == x)
			{
				height_map_to[i][j] += height;
			}*/
		}
	}

	CreateNormals();

	
	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			if (bPlaneUpdate[i][j] == true)
			{
				m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map_to, normals, colours, steepness, tex_blend);
			}
		}
	}
	
}

void Level::CreateNormals()
{
	for (int j = 0; j < total_y_points; j++)
	{
		for (int i = 0; i < total_x_points; i++)
		{


			float tl = height_map_to[clampB(j - 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float t = height_map_to[clampB(j - 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float tr = height_map_to[clampB(j - 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];// m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j - 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float r = height_map_to[clampB(j, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float br = height_map_to[clampB(j + 1, total_y_points - 1)][clampB(i + 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i + 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float b = height_map_to[clampB(j + 1, total_y_points - 1)][clampB(i, total_x_points - 1)];//m_hminfo.heightMap[clampB(i, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float bl = height_map_to[clampB(j + 1, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j + 1), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;
			float l = height_map_to[clampB(j, total_y_points - 1)][clampB(i - 1, total_x_points - 1)];//m_hminfo.heightMap[clampB(i - 1, m_hminfo.terrainWidth) + (clampB((j), m_hminfo.terrainHeight)*m_hminfo.terrainWidth)].y;

			float dZ = -((tr + (1.5f * r) + br) - (tl + (1.5f * l) + bl));
			float dX = (tl + (1.5f * t) + tr) - (bl + (1.5f * b) + br);
			float dY = 35.5;// This is for car physics plane

			btVector3 v(dX, dY, dZ);
			v.normalize();

			normals[j][i].x = (float)v.getX();
			normals[j][i].y = (float)v.getY();
			normals[j][i].z = (float)v.getZ();

			dY = m_Resources->m_LevelInfo.ground_steepness_blend;
			v = btVector3(dX, dY, dZ);
			v.normalize();
			steepness[j][i] = 1.0f-v.getY();
		}
	}
	//SetupTerrainColours();
}

//void Level::InitializeTerrain( char* terrain_filename,wchar_t* texture_filename, Physics* phys , float height_scale)
void Level::UpdateGeneratedTerrain(int level, float noise_z)
{


	unsigned int seed = 237;
	float scale = 0.05f;
	PerlinNoise pn(seed);

	for (int i = 0; i <total_y_points; i++) {
		

		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = (pn.noise((float)j*scale+ noise_z, (float)i*scale, noise_z*0.1f))*20.0f;//*
																 //pn.noise((float)j*scale*3.0f, (float)i*scale*3.0f, 1.0f))*20.0f;
		}
	}

	CreateNormals();

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_Terrain[i + (j*x_planes)]->UpdateFromHeightMap(height_map, normals, colours, steepness, tex_blend);

		}
	}
}


void Level::SetWaterActive()
{
	//concurrency::task<void>

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_Water[i + (j*x_planes)]->bActive = false;
			m_Water[i + (j*x_planes)]->LoadTerrainHeight(height_map);
			for (float k = 0; k < 24; k+=1.0f)
			{
				for (float l = 0; l < 24; l += 1.0f)
				{

					if (GetTerrainHeight(m_Water[i + (j*x_planes)]->xpos - 12.0f+k, m_Water[i + (j*x_planes)]->zpos - 12.0f+l) < 1.0f)
					{
						m_Water[i + (j*x_planes)]->bActive = true;
					}
				}
			}

			if (m_Water[i + (j*x_planes)]->bActive == true)
			{
				m_Water[i + (j*x_planes)]->GetActiveVerts(wave_active_map);
			}
		}
	}

}


/*
void Level::SetVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_vertices.data(), 0, 0 };
	UINT bytes = sizeof(GroundVertexData) * m_vertices.size();
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_vertexBuffer));
}


void Level::SetIndexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indices.size();
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer));
}
*/

void Level::UpdateWaveheightmap(float timeDelta, float timeTotal)
{

	unsigned int seed = 237;

	PerlinNoise pn(seed);
	float noise_z = 0.5f;

	float change, x_movement, z_movement;

	x_movement = -(timeTotal*m_Resources->m_LevelInfo.wind.x)*0.2f;
	z_movement = (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.2f;

	change = timeTotal*0.05f;

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_Water[i + (j*x_planes)]->cam_check_point = m_Resources->m_Camera->CheckPoint(m_Water[i + (j*x_planes)]->xpos, 0.0f, m_Water[i + (j*x_planes)]->zpos, 17.0f, &m_Terrain[i + (j*x_planes)]->cam_check_point_full);
			m_Terrain[i + (j*x_planes)]->cam_check_point = m_Water[i + (j*x_planes)]->cam_check_point;

		}
	}


	for (int i = 0; i < total_y_points; i++)
	{
		//height_map[i] = new float[total_x_points];

		for (int j = 0; j < total_x_points; j++)
		{
			//if (m_Water[(j/8) + ((i/8)*x_planes)]->cam_check_point > 0.0f)
			//{
				if (wave_active_map[i][j] == true)
				{
					wave_height_map[i][j] = (pn.noise(((float)(j)+x_movement)*m_Resources->m_LevelInfo.wind.w, ((float)(i)+z_movement)*m_Resources->m_LevelInfo.wind.w, change))*m_Resources->m_LevelInfo.wind.y;
				}
			//}
		}
	}
}

Concurrency::task<void> Level::Update(float timeDelta, float timeTotal)
{
	current_index = 0;
	
	std::vector<concurrency::task<void>> tasks;

	//m_vertices.clear();
	//m_indices.clear();
	UpdateWaveheightmap(timeDelta, timeTotal);

	p_index.clear();

	part_index pi;
	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{

			//m_Water[i + (j*x_planes)]->cam_check_point = m_Resources->m_Camera->CheckPoint(m_Water[i + (j*x_planes)]->xpos, 0.0f, m_Water[i + (j*x_planes)]->zpos, 17.0f, &m_Terrain[i + (j*x_planes)]->cam_check_point_full);
			//m_Terrain[i + (j*x_planes)]->cam_check_point = m_Water[i + (j*x_planes)]->cam_check_point;

			m_Terrain[i + (j*x_planes)]->cam_check_point = m_Resources->m_Camera->CheckPoint(m_Terrain[i + (j*x_planes)]->xpos, m_Terrain[i + (j*x_planes)]->average_height, m_Terrain[i + (j*x_planes)]->zpos, m_Terrain[i + (j*x_planes)]->furthest_point, &m_Terrain[i + (j*x_planes)]->cam_check_point_full);

			if (m_Terrain[i + (j*x_planes)]->cam_check_point > 0.0f)
			{

				tasks.push_back(m_Water[i + (j*x_planes)]->Update(timeDelta, timeTotal,wave_height_map));

				m_Terrain[i + (j*x_planes)]->b_in_scene = true;



				pi.part_no = i + (j*x_planes);
				pi.dist = m_Terrain[i + (j*x_planes)]->cam_check_point;

				p_index.push_back(pi);
				//p_index[current_index].part_no = i + (j*x_planes);
				//p_index[current_index].dist = m_Terrain[i + (j*x_planes)]->cam_check_point;
				current_index++;

				m_Terrain[i + (j*x_planes)]->m_rigidbody->setCollisionFlags(!btCollisionObject::CF_NO_CONTACT_RESPONSE);
			}
			else
			{
				m_Terrain[i + (j*x_planes)]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

				m_Terrain[i + (j*x_planes)]->b_in_scene = false;
			}

		}
	}

	tasks.push_back(concurrency::create_task([this]
	{
		std::sort(p_index.begin(), p_index.end(), terrain_compare{});
		//qsort(p_index, current_index, sizeof(part_index), TerrainSortCB);
	}));
	// copy terrain data into one buffer pair to render faster
	/*
	tasks.push_back(concurrency::create_task([this]
	{
		GroundVertexData vdata;
		unsigned short idata;
		unsigned short vindex = m_vertices.size();

		for (int j = 0; j < y_planes; j++)
		{
			for (int i = 0; i < x_planes; i++)
			{
				Terrain* t = m_Terrain[i + (j*x_planes)];
				if (t->b_in_scene == true)
				{
					vindex = m_vertices.size();

					for (int k = 0; k < t->m_vertices.size(); k++)
					{
						vdata = t->m_vertices.at(k);
						vdata.pos.x += t->pos_x;
						vdata.pos.y += t->pos_y;
						vdata.pos.z += t->pos_z;
						m_vertices.push_back(vdata);
					}

					for (int k = 0; k < t->m_indices.size(); k++)
					{
						m_indices.push_back(t->m_indices.at(k)+vindex);
					}
				}
			}
		}
		//m_Resources->m_Physics.Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
	}));
	*/



	return when_all(begin(tasks), end(tasks));

	
}




// Sets up the arrays
void Level::InitializeTerrainVariables(int x_nodes, int y_nodes, float terrain_scale)
{
	x_planes = x_nodes;
	y_planes = y_nodes;

	local_terrain_scale = terrain_scale;

	total_x_points = (x_planes * 8) + 1;
	total_y_points = (y_planes * 8) + 1;


	for (int i = 0; i < x_planes*y_planes; i++)
	{
		m_Terrain[i] = new Terrain(m_Resources, 1);
	}


	for (int i = 0; i < x_planes*y_planes; i++)
	{
		m_Water[i] = new Water(m_Resources, 1);
	}

	//m_Paths->InitArrays(total_x_points, total_y_points);


	tex_blend = new float**[6];
	for (int i = 0; i <6; i++)
	{
		tex_blend[i] = new float*[total_y_points];
		for (int j = 0; j < total_y_points; j++)
		{
			tex_blend[i][j] = new float[total_x_points];
		}
	}

	m_Paths->SetBlendArray(tex_blend[0]);







	colours = new XMFLOAT4*[total_y_points];
	for (int i = 0; i < total_y_points; i++)
	{
		colours[i] = new XMFLOAT4[total_x_points];
	}

	wave_height_map = new float*[total_y_points];
	for (int i = 0; i <total_y_points; i++) 
	{
		wave_height_map[i] = new float[total_x_points];
		for (int j = 0; j < total_x_points; j++)
		{
			wave_height_map[i][j] = 0.0f;
		}
	}


	wave_active_map = new bool*[total_y_points];
	for (int i = 0; i <total_y_points; i++)
	{
		wave_active_map[i] = new bool[total_x_points];
		for (int j = 0; j < total_x_points; j++)
		{
			wave_active_map[i][j] = false;
		}
	}


	height_map = new float*[total_y_points];
	for (int i = 0; i <total_y_points; i++)
	{
		height_map[i] = new float[total_x_points];
	}

	

	steepness = new float*[total_y_points];
	for (int i = 0; i <total_y_points; i++)
	{
		steepness[i] = new float[total_x_points];
	}


	height_map_to = new float*[total_y_points];
	for (int i = 0; i < total_y_points; i++)
	{
		height_map_to[i] = new float[total_x_points];
	}

	bPlaneUpdate = new bool*[total_y_points];

	for (int i = 0; i <total_y_points; i++) 
	{
		bPlaneUpdate[i] = new bool[total_x_points];
		for (int j = 0; j < total_x_points;j++)
		{
			bPlaneUpdate[i][j] = false;
		}
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
			m_Terrain[i + (j*x_planes)]->InitTerrain(i * 8, j * 8);

			m_Water[i + (j*x_planes)]->InitWater(i * 8, j * 8);

			float shift = 160.0f;

			float posx = -(shift*terrain_scale) + (((float)(i))*(shift*terrain_scale));
			float posz = -(shift*terrain_scale) + (((float)(j))*(shift*terrain_scale));

			posx += ((80.0f - (80.0f*(x_planes - 2)))*terrain_scale); // for 2
			posz += ((80.0f - (80.0f*(y_planes - 2)))*terrain_scale); // for 2

			m_Terrain[i + (j*x_planes)]->SetPosition(posx, 0.0f, posz);
			m_Water[i + (j*x_planes)]->SetPosition(posx, 0.0f, posz);

			m_Terrain[i + (j*x_planes)]->zpos = posz;
			m_Terrain[i + (j*x_planes)]->xpos = posx;
			m_Terrain[i + (j*x_planes)]->cam_z = cam_z;
			total_planes++;
		}
	}
	
}

void Level::MakeNewTerrain(float val1, float val2, float val3, float val4)
{
	unsigned int seed = 237;
	float scale = 0.08f/val2;;
	PerlinNoise pn(seed);

	for (int i = 0; i < total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			colours[i][j] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}


	for (int i = 0; i <total_y_points; i++)
	{
		for (int j = 0; j < total_x_points; j++)
		{
			height_map_to[i][j] = ((pn.noise((float)j*scale, (float)i*scale, val4))*(val1*0.4f))+(val2*0.1f);//*
			height_map_to[i][j] -= ((pn.noise((float)j*(scale*2.0f), (float)i*(scale*2.0f), val4))*(val1*0.4f)) + (val2*0.1f);//*

			if (height_map_to[i][j] < 0.0f)
				height_map_to[i][j] *= 0.1f;
		}
	}

	//CalculateNormals();

	/*
	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_Terrain[i + (j*x_planes)]->LoadFromHeightMap(height_map, normals, colours, 9, 9, (20.0f*local_terrain_scale), i, steepness, m_Paths->tex_blend);
			m_Terrain[i + (j*x_planes)]->SetPosition(m_Terrain[i + (j*x_planes)]->xpos, 0.0f, m_Terrain[i + (j*x_planes)]->zpos);

			m_Water[i + (j*x_planes)]->SetPosition(m_Terrain[i + (j*x_planes)]->xpos, 0.0f, m_Terrain[i + (j*x_planes)]->zpos);



			m_Water[i + (j*x_planes)]->SetFlat(9, 9, (20.0f*local_terrain_scale), i);

			//m_Water[i + (j*x_planes)]->LoadTerrainHeight(height_map);

		}
	}
	*/
}

void Level::InitializeGeneratedTerrain(int level, float height_scale, float noise_z)
{
	wchar_t texture_filename[40];
	wchar_t texture_filename_2[40];
	wchar_t texture_filename_3[40];
	wchar_t texture_filename_norm[40];

	wchar_t ground_filename1[30];
	wchar_t ground_filename2[30];
	wchar_t ground_filename3[30];
	wchar_t ground_filename_norm[30];

	unsigned int seed = 237;
	float scale = 0.08f;
	PerlinNoise pn(seed);

	for (int i = 0; i < total_y_points; i++) 
	{
		for (int j = 0; j < total_x_points; j++)
		{
			colours[i][j] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	

	for (int i = 0; i <total_y_points; i++) 
	{
		for (int j = 0; j < total_x_points; j++)
		{
			height_map[i][j] = (pn.noise((float)j*scale, (float)i*scale, noise_z))*20.0f;//*
		}
	}

	CreateNormals();


	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			m_Terrain[i + (j*x_planes)]->LoadFromHeightMap(height_map,normals, colours,9, 9, (20.0f*local_terrain_scale), i, steepness, tex_blend);
			m_Terrain[i + (j*x_planes)]->SetPosition(m_Terrain[i + (j*x_planes)]->xpos, 0.0f, m_Terrain[i + (j*x_planes)]->zpos);

			m_Water[i + (j*x_planes)]->SetPosition(m_Terrain[i + (j*x_planes)]->xpos, 0.0f, m_Terrain[i + (j*x_planes)]->zpos);



			m_Water[i + (j*x_planes)]->SetFlat(9, 9, (20.0f*local_terrain_scale), i);

			//m_Water[i + (j*x_planes)]->LoadTerrainHeight(height_map);

		}
	}

	
	/*
	mbstowcs(ground_filename1, m_Resources->m_LevelInfo.terrain_tex1, 30);
	mbstowcs(ground_filename2, m_Resources->m_LevelInfo.terrain_tex2, 30);
	mbstowcs(ground_filename3, m_Resources->m_LevelInfo.terrain_tex3, 30);

	mbstowcs(ground_filename_norm, m_Resources->m_LevelInfo.terrain_normal, 30);


	swprintf(texture_filename, 40, L"Assets/Ground/%s.dds", ground_filename1);
	swprintf(texture_filename_2, 40, L"Assets/Ground/%s.dds", ground_filename2);
	swprintf(texture_filename_3, 40, L"Assets/Ground/%s.dds", ground_filename3);

	swprintf(texture_filename_norm, 40, L"%s.dds", ground_filename_norm);

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename, nullptr, &m_Texture, MAXSIZE_T);
	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename_2, nullptr, &m_Texture_2, MAXSIZE_T);
	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename_3, nullptr, &m_Texture_3, MAXSIZE_T);

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), texture_filename_norm, nullptr, &m_Texture_Normal, MAXSIZE_T);
	*/
}

void Level::MakePhysics()
{
	left_right_walls = 8.0f * (float)x_planes;
	front_back_walls = 8.0f * (float)y_planes;

	for (int j = 0; j < y_planes; j++)
	{
		for (int i = 0; i < x_planes; i++)
		{
			if (j>2 && j<20 && i>2 && i<20)
			{
				m_Terrain[i + (j*x_planes)]->MakePhysicsConvexTriangleTerrain();
			}
		}
	}



	ObjInfo wall_info;
	wall_info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS);
	wall_info.mask = (COL_WALLS | COL_RAY);
	wall_info.mrf = XMFLOAT3(1.0f, 0.9f, 0.1f);







	/*
	btDefaultMotionState* wall1 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(left_right_walls + 1.8f, -1, 0)));
	auto wall1Shape = new btStaticPlaneShape(btVector3(-1, 0, 0), 1);//left
	m_Resources->m_Physics.AddPhysicalObject(wall1Shape, wall1, btVector3(0, 0, 0), &wall_info);

	btDefaultMotionState* wall2 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-left_right_walls + 0.2f, -1, 0)));
	auto wall2Shape = new btStaticPlaneShape(btVector3(1, 0, 0), 1);//right
	m_Resources->m_Physics.AddPhysicalObject(wall2Shape, wall2, btVector3(0, 0, 0), &wall_info);

	btDefaultMotionState* wall3 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.0f, -1.0f, front_back_walls - 0.2f)));
	auto wall3Shape = new btStaticPlaneShape(btVector3(0, 0, -1), 1);//back
	m_Resources->m_Physics.AddPhysicalObject(wall3Shape, wall3, btVector3(0, 0, 0), &wall_info);

	btDefaultMotionState* wall4 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.0f, -1.0f, -front_back_walls + 0.2f)));
	auto wall4Shape = new btStaticPlaneShape(btVector3(0, 0, 1), 1);//front
	m_Resources->m_Physics.AddPhysicalObject(wall4Shape, wall4, btVector3(0, 0, 0), &wall_info);
	*/

	wall_info.pos = XMFLOAT3(1.8f, 10.0f, 0.0f);
	wall_info.dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_LeftPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);
	// left
	//wall_info.pos = XMFLOAT3(left_right_walls + 1.8f, -1.0f, 0.0f);
	//wall_info.dir = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	//m_LeftPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);
	// right
	wall_info.pos = XMFLOAT3(-left_right_walls + 0.2f, -1.0f, 0.0f);
	wall_info.dir = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_RightPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);
	// back
	wall_info.pos = XMFLOAT3(0.0f, -1.0f, front_back_walls - 0.2f);
	wall_info.dir = XMFLOAT3(0.0f, 0.0f, -1.0f);
	m_BackPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);
	// front
	wall_info.pos = XMFLOAT3(0.0f, -1.0f, -front_back_walls + 0.2f);
	wall_info.dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_FrontPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);
	// ground
	
	///wall_info.pos = XMFLOAT3(0, -1, 0);
	////wall_info.dir = XMFLOAT3(0, 1, 0);
	//m_Resources->m_Physics.MakeWallPlane(&wall_info);
	
	// floor plane
	//wall_info.pos = XMFLOAT3(0, 1.0f, 0.0f);
	//wall_info.dir = XMFLOAT3(0, 1, 0);
	//m_FloorPlane = m_Resources->m_Physics.MakeWallPlane(&wall_info);


}


Level::~Level(void)
{
	ClearMemory();
}

void Level::ClearMemory()
{
	int i;
	//height_map = new float*[total_y_points];
#ifndef _DEBUG
	/*
	if (m_Texture != nullptr)
		m_Texture->Release();
	if (m_Texture_2 != nullptr)
		m_Texture_2->Release();
		*/
#endif // !_DEBUG


	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < total_y_points; j++)
		{
			delete[] tex_blend[i][j];// = new float[total_x_points];
		}
		delete[] tex_blend[i];
	}
	delete[] tex_blend;


	for (i = 0; i <total_y_points; i++) {
		delete[] height_map[i]; // = new float[total_x_points];
	}
	delete[] height_map;

	for (i = 0; i <total_y_points; i++) {
		delete[] wave_height_map[i]; // = new float[total_x_points];
	}
	delete[] wave_height_map;


	for (i = 0; i <total_y_points; i++) {
		delete[] wave_active_map[i]; // = new float[total_x_points];
	}
	delete[] wave_active_map;


	for (i = 0; i <total_y_points; i++) {
		delete[] steepness[i]; // = new float[total_x_points];
	}
	delete[] steepness;	//normals = new norm_t*[total_y_points];

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
		m_Terrain[i]->ClearMemory();
	}


	//delete[] m_hminfo.heightMap;
	//delete[] m_hminfo.normal;
	//delete[] m_hminfo.colour;
	total_planes = 0;



}
