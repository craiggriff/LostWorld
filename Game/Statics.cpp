
#include "pch.h"
#include "Statics.h"
#include "DefPhysics.h"

using namespace Game;
using namespace DirectX;



#define MAX_STATICS 5000


// std::sort with inlining
struct statics_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

// std::sort with inlining
struct statics_car_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist > b.dist;
	}
};




void Statics::Reset()
{
	cur_total = 0;


	current_selected_index = 0;

	for (int i = 0; i < MAX_STATICS; i++)
	{
		m_static[i].bActive = 0;

	}

}


Statics::Statics(AllResources* p_Resources)
{
	int i;
	m_Resources = p_Resources;

	m_deviceResources = p_Resources->m_deviceResources;

	max_num = 200;



	m_Motion = new MeshMotion*[MAX_STATICS];


	for (i = 0; i < MAX_STATICS; i++)
	{
		m_Motion[i] = nullptr;
	}


	m_static = new static_t[MAX_STATICS];
	
	for (i = 0; i < MAX_STATICS; i++)
	{
		m_static[i].bActive = 0;
	}


	for (i = 0; i < 50; i++)
	{
		m_static_model[i] = new static_model_t;
	}

	p_index.clear();
	p_index_car.clear();

	cur_total = 0;


	m_Flag = new Flag(m_Resources);
	m_Flag->InitializeTerrainVariables(2, 2, 0.01f);
	m_Flag->InitializeGeneratedTerrain(0, 6.0f, 0.0f);
	m_Flag->m_Texture = m_Resources->m_Textures->LoadTexture("masontemplar");

	bRandomness = false;

	current_selected_index = 0;

	m_skinnedMeshRenderer.Initialize(m_Resources->m_deviceResources->GetD3DDevice(), m_Resources->m_deviceResources->GetD3DDeviceContext(), m_Resources->m_materialBuffer);


	bLoadingComplete = false;

	bMakePhysicsWhenLoaded = false;
}

//void Statics::Rotate

void Statics::Clear(float x, float z)
{
	for (int i = 0; i < cur_total; i++)
	{
		if (m_static[i].bActive == 1 &&
			m_static[i].x == x &&
			m_static[i].z == z)
		{
			if (i == cur_total-1)
			{
				cur_total--;
			}
			else
			{
				if (cur_total-1 > 0)
				{
					memcpy(&m_static[i] , &m_static[cur_total-1],sizeof(static_t));
					//m_static[i].model_matrix = m_static[cur_total].model_matrix;
					cur_total--;
				}
				else
				{
					cur_total = 0;
				}
			}
		}
	}
}

void Statics::CreateOne(float x, float y, float z,float height_from_t,float _scale,float rx,float ry,float rz)
{
	int i;
	bool bFound = false;

	if (cur_total >= MAX_STATICS)
		return;

	for (i = 0; i < cur_total; i++)
	{
		if (m_static[i].bActive == 1 &&
			m_static[i].x == x &&
			m_static[i].z == z)
			bFound = true;
	}

	if (bRandomness == true)
	{
		ry = ((rand() % 1000)*0.001)*(M_PI*2.0f);
	}

	if (bFound == false)
	{
		
		m_static[cur_total].bActive = 1;
		m_static[cur_total].type = m_static_model[current_selected_index]->type;
		m_static[cur_total].height_from_terrain = height_from_t;
		m_static[cur_total].x = x;
		m_static[cur_total].y = y;
		m_static[cur_total].z = z;
		m_static[cur_total].rot_x = rx + m_static_model[current_selected_index]->info.dir.x;
		m_static[cur_total].rot_y = ry + m_static_model[current_selected_index]->info.dir.y;
		m_static[cur_total].rot_z = rz + m_static_model[current_selected_index]->info.dir.z;
		m_static[cur_total].model_index = current_selected_index;
		//m_static[cur_total].model_matrix = m_Resources->MakeMatrix(x, y, z, rx, ry, rz, 1.0f);
		m_static[cur_total].scale = _scale;
		m_static[cur_total].bActive = 1;
		m_static[cur_total].flag_pos = (rand() % 100)*0.1f;// 0.0f;
		//m_static[cur_total].bHasFlag = false;

		m_static[cur_total].colour = XMFLOAT4(float(m_Resources->col_r) / 256.0f, float(m_Resources->col_g) / 256.0f, float(m_Resources->col_b) / 256.0f, 1.0f);

		m_static[cur_total].MakeMatrix();
		//if (current_selected_index == 2)
	//	{
		//	m_static[cur_total].bHasFlag = true;
		//}
		cur_total++;
	}
}



void Statics::MakePhysics()
{
	int i;

	if (bLoadingComplete == false)
	{
		bMakePhysicsWhenLoaded = true;
		return;
	}

	cur_phy = 0;
	//phy = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics);

	for (i = 0; i < cur_total; i++)
	{
		switch (m_static_model[m_static[i].model_index]->physics_shape)
		{
		case 0:break;
		case 1:

			m_Motion[cur_phy] = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics);

			
			m_Motion[cur_phy]->ob_info = m_static_model[m_static[i].model_index]->info;
			m_Motion[cur_phy]->ob_info.pos = XMFLOAT3(m_static[i].x, m_static[i].y+0.9f, m_static[i].z);
			m_Motion[cur_phy]->ob_info.dir = XMFLOAT3(m_static[i].rot_x, m_static[i].rot_y, m_static[i].rot_z);
			

			//m_Motion[cur_phy]->MakePhysicsBoxFromFBX(m_static_model[m_static[i].model_index]->m_meshModels[0], 1.0f);
			m_Motion[cur_phy]->MakePhysicsBoxExtents(0.8f,0.8f,0.8f, 1.0f);
			m_Motion[cur_phy]->MakeMatrix();
			m_Motion[cur_phy++]->static_index = i;
			break;
		case 2:

			m_Motion[cur_phy] = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics);

			m_Motion[cur_phy]->ob_info = m_static_model[m_static[i].model_index]->info;
			m_Motion[cur_phy]->ob_info.pos = XMFLOAT3(m_static[i].x, m_static[i].y + 2.0f, m_static[i].z);
			m_Motion[cur_phy]->ob_info.dir = XMFLOAT3(m_static[i].rot_x, m_static[i].rot_y, m_static[i].rot_z);


			//m_Motion[cur_phy]->MakePhysicsBoxFromFBX(m_static_model[m_static[i].model_index]->m_meshModels[0], 1.0f);
			m_Motion[cur_phy]->MakePhysicsBoxExtents(0.5f, 2.0f, 0.5f, 1.0f);
			m_Motion[cur_phy]->MakeMatrix();
			m_Motion[cur_phy++]->static_index = i;
			break;
		case 3:

			m_Motion[cur_phy] = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics);

			m_Motion[cur_phy]->ob_info = m_static_model[m_static[i].model_index]->info;
			m_Motion[cur_phy]->ob_info.pos = XMFLOAT3(m_static[i].x, m_static[i].y + 2.0f, m_static[i].z);
			m_Motion[cur_phy]->ob_info.dir = XMFLOAT3(m_static[i].rot_x, m_static[i].rot_y, m_static[i].rot_z);


			//m_Motion[cur_phy]->MakePhysicsBoxFromFBX(m_static_model[m_static[i].model_index]->m_meshModels[0], 1.0f);
			m_Motion[cur_phy]->MakePhysicsBoxExtents(0.5f, 2.0f, 0.5f, 1.0f);
			m_Motion[cur_phy]->MakeMatrix();
			m_Motion[cur_phy++]->static_index = i;
			break;
		default:
			break;
		}
	}
}

void Statics::LoadModels()
{
	std::vector<concurrency::task<void>> tasks;

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources,L"tree.cmo", L"", L"", m_static_model[0]->m_meshModels).then([this]()
	{
		m_static_model[0]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("tree_leaves"),1);
		m_static_model[0]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("tree_bark"),0);

		//m_static_model[3]->m_meshModels[1]->SetMaterialNormal(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("metall001-new-nm"),1.0f);
		m_static_model[0]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		m_static_model[0]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_static_model[0]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[0]->type = 0;
		m_static_model[0]->physics_shape = 3;
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"cp.cmo", L"", L"", m_static_model[1]->m_meshModels).then([this]()
	{
		m_static_model[1]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("inksplat"), 1);
		m_static_model[1]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("marttex"), 0);
		m_static_model[1]->type = 0;
		m_static_model[1]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"flag-pole.cmo", L"", L"", m_static_model[2]->m_meshModels).then([this]()
	{
		m_static_model[2]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("tree_bark"), 0);
		m_static_model[2]->type = 0;
		m_static_model[2]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"lamp_post.cmo", L"", L"", m_static_model[3]->m_meshModels).then([this]()
	{
		//m_static_model[3]->m_meshModels[0]->SetMaterialTexture(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("yell"), 1);
		m_static_model[3]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("lamppost"), 0);
		m_static_model[3]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("lamp-glass"), 2); 

		//m_static_model[3]->m_meshModels[1]->SetMaterialNormal(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("metall001-new-nm"),1.0f);
		m_static_model[3]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		m_static_model[3]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_static_model[3]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[3]->type = 0;
		m_static_model[3]->light_radius = 24.0f;
		m_static_model[3]->physics_shape = 2;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"fireplace.cmo",	L"", L"", m_static_model[4]->m_meshModels).then([this]()
	{
		m_static_model[4]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("fireplace_tex"), 0);
		m_static_model[4]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("fireplace_uv"), 1.0f);

		m_static_model[4]->type = 0;

		m_static_model[4]->light_radius = 24.0f;
		m_static_model[4]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"mushroom.cmo", L"", L"", m_static_model[5]->m_meshModels,1.0f).then([this]()
	{
		m_static_model[5]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("mushroomred"),0);
		m_static_model[5]->type = 1;
		m_static_model[5]->physics_shape = 0;

	}));
	
	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"heart.cmo", L"", L"", m_static_model[6]->m_meshModels, 0.5f).then([this]()
	{
		m_static_model[6]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("heart"), 0);
		m_static_model[6]->type = 1;
		m_static_model[6]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"gem2.cmo", L"",	L"", m_static_model[7]->m_meshModels, 0.5f).then([this]()
	{
		m_static_model[7]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("gemwhite"), 2);
		m_static_model[7]->type = 1;
		m_static_model[7]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"gem3.cmo", L"", L"", m_static_model[8]->m_meshModels, 0.5f).then([this]()
	{
		m_static_model[8]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("gemblue"),2);
		m_static_model[8]->type = 1;
		m_static_model[8]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"gem1.cmo", L"", L"", m_static_model[9]->m_meshModels, 0.5f).then([this]()
	{
		m_static_model[9]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("gemred"), 2);
		m_static_model[9]->type = 1;
		m_static_model[9]->physics_shape = 0;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"trees_0.cmo", L"", L"", m_static_model[10]->m_meshModels).then([this]()
	{
		m_static_model[10]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_col"), 0);
		m_static_model[10]->m_meshModels[1]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_nor"),2.0f);
		
		m_static_model[10]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_leaves_003_col"), 1);

		//m_static_model[3]->m_meshModels[1]->SetMaterialNormal(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("metall001-new-nm"),1.0f);
		m_static_model[10]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		m_static_model[10]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_static_model[10]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[10]->type = 0;
		m_static_model[10]->physics_shape = 2;

	}));


	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"star.cmo", L"", L"", m_static_model[11]->m_meshModels, 0.5f).then([this]()
	{
		m_static_model[11]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("yell"), 0);
		m_static_model[11]->type = 1;
		m_static_model[11]->physics_shape = 0;

	}));


	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"hedge1.cmo", L"", L"", m_static_model[12]->m_meshModels).then([this]()
	{
		m_static_model[12]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("hedge_diffuse"), 1);
		m_static_model[12]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("hedge_normal"), 1.0f);

		m_static_model[12]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		m_static_model[12]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_static_model[12]->info.mask = (COL_OBJECTS | COL_RAY);
		
		m_static_model[12]->type = 0;
		m_static_model[12]->physics_shape = 1;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"hedge2.cmo", L"", L"", m_static_model[13]->m_meshModels).then([this]()
	{
		m_static_model[13]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("hedgelong_diffuse"), 1);
		m_static_model[13]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("hedgelong_normal"), 1.0f);


		m_static_model[13]->type = 0;
		m_static_model[13]->physics_shape = 0;

	}));


	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"hedgearch.cmo", L"", L"", m_static_model[14]->m_meshModels).then([this]()
	{
		m_static_model[14]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("hedgearch_diffuse"), 1);
		m_static_model[14]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("hedgearch_normal"), 1.0f);


		m_static_model[14]->type = 0;
		m_static_model[14]->physics_shape = 0;

	}));
	
	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"hedgeround.cmo", L"", L"", m_static_model[15]->m_meshModels).then([this]()
	{
		m_static_model[15]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("hedgearch_diffuse"), 1);
		m_static_model[15]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("hedgearch_normal"), 1.0f);


		m_static_model[15]->type = 0;
		m_static_model[15]->physics_shape = 0;

	}));


	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"mikey.cmo", L"", L"", m_static_model[16]->m_meshModels,0.5f).then([this]()
	{
		//m_static_model[16]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_col"), 0);
		//m_static_model[16]->m_meshModels[1]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_nor"), 2.0f);

		m_static_model[16]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_leaves_003_col"), 1);

		m_static_model[16]->type = 0;
		m_static_model[16]->info.dir.x = -0.5f * M_PI;
		m_static_model[16]->info.dir.y = 0.0f;
		m_static_model[16]->info.dir.z = 0.5f * M_PI;
		m_static_model[16]->physics_shape = 0;

		//m_static_model[16]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[16]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[16]->info.mask = (COL_OBJECTS | COL_RAY);

		//m_static_model[16]->type = 0;
		//m_static_model[16]->physics_shape = 2;

	}));


	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"statue1.cmo", L"", L"", m_static_model[17]->m_meshModels, 0.2f).then([this]()
	{
		//m_static_model[16]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_col"), 0);
		//m_static_model[16]->m_meshModels[1]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_nor"), 2.0f);

		m_static_model[17]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("gemwhite"), 1);

		m_static_model[17]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		m_static_model[17]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_static_model[17]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[17]->type = 0;
		m_static_model[17]->physics_shape = 3;

		//m_static_model[16]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[16]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[16]->info.mask = (COL_OBJECTS | COL_RAY);

		//m_static_model[16]->type = 0;
		//m_static_model[16]->physics_shape = 2;

	}));

	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"big_old_house.cmo", L"", L"", m_static_model[18]->m_meshModels, 3.0f).then([this]()
	{
		//m_static_model[16]->m_meshModels[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_col"), 0);
		//m_static_model[16]->m_meshModels[1]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("trees_bark_002_nor"), 2.0f);

		m_static_model[18]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("big_old_house_c"), 1);

		//m_static_model[17]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[17]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[17]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[18]->type = 0;
		m_static_model[18]->physics_shape = 0;

		//m_static_model[16]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[16]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[16]->info.mask = (COL_OBJECTS | COL_RAY);

		//m_static_model[16]->type = 0;
		//m_static_model[16]->physics_shape = 2;

	}));

		tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"lhouse.cmo", L"", L"", m_static_model[19]->m_meshModels, 2.0f).then([this]()
	{
		m_static_model[19]->m_meshModels[0]->SetMaterialTexture(L"SR_White", 0, m_Resources->m_Textures->LoadTexture("3td_WhiteBrick_02"), 0);
		m_static_model[19]->m_meshModels[0]->SetMaterialNormal(L"SR_White", 0, m_Resources->m_Textures->LoadTexture("3td_WhiteBrick_02_NRM"), 2.0f);

		m_static_model[19]->m_meshModels[0]->SetMaterialTexture(L"SR_TOP", 0, m_Resources->m_Textures->LoadTexture("3td_WhiteBrick_02"), 0);
		m_static_model[19]->m_meshModels[0]->SetMaterialNormal(L"SR_TOP", 0, m_Resources->m_Textures->LoadTexture("3td_WhiteBrick_02_NRM"), 2.0f);

		m_static_model[19]->m_meshModels[0]->SetMaterialTexture(L"SR_Brick", 0, m_Resources->m_Textures->LoadTexture("3td_Stone_08"), 0);
		m_static_model[19]->m_meshModels[0]->SetMaterialNormal(L"SR_Brick", 0, m_Resources->m_Textures->LoadTexture("3td_Stone_08_NRM"), 2.0f);

		m_static_model[19]->m_meshModels[0]->SetMaterialTexture(L"SR_Door", 0, m_Resources->m_Textures->LoadTexture("door01"), 0);
		m_static_model[19]->m_meshModels[0]->SetMaterialNormal(L"SR_Door", 0, m_Resources->m_Textures->LoadTexture("door01_NRM"), 2.0f);


		//m_static_model[19]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("big_old_house_c"), 1);

		//m_static_model[17]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[17]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[17]->info.mask = (COL_OBJECTS | COL_RAY);

		m_static_model[19]->type = 0;
		m_static_model[19]->physics_shape = 0;

		//m_static_model[16]->info.mrf = XMFLOAT3(0.0f, 0.5f, 0.5f);
		//m_static_model[16]->info.group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		//m_static_model[16]->info.mask = (COL_OBJECTS | COL_RAY);

		//m_static_model[16]->type = 0;
		//m_static_model[16]->physics_shape = 2;

	}));




	/*
	tasks.push_back(Mesh::LoadFromFileAsync(*m_Resources, L"hedgeround.cmo", L"", L"", m_static_model[15]->m_meshModels).then([this]()
	{
		m_static_model[15]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("hedgeround_diffuse"), 2); <- problem with these textures
		m_static_model[15]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("hedgeround_normal"), 1.0f);


		m_static_model[15]->type = 0;
	}));
	*/
	auto joinTask = when_all(begin(tasks), end(tasks));

	joinTask.then([this]
	{

		for (int i = 0; i < 17; i++)
		{
			// Initialize animated models.
			for (Mesh* m : m_static_model[i]->m_meshModels)
			{

				if (m->BoneInfoCollection().empty() == false)
				{
					m_static_model[i]->bHasAnim = true;
					auto animState = new AnimationState();
					animState->m_boneWorldTransforms.resize(m->BoneInfoCollection().size());
					animState->m_boneWorldTransformsB.resize(m->BoneInfoCollection().size());
					animState->m_boneWorldTransformsC.resize(m->BoneInfoCollection().size());
					animState->m_boneWorldTransformsD.resize(m->BoneInfoCollection().size());
					m->Tag = animState;
				}
			}

			// Each mesh object has its own "time" used to control the glow effect.
			m_static_model[i]->m_time.clear();
			for (size_t i = 0; i < m_static_model[i]->m_meshModels.size(); i++)
			{
				m_static_model[i]->m_time.push_back(0.0f);
			}
		}


		bLoadingComplete = true;
		//MakePhysics();
	});


	total_static_models = 12;
}


void Statics::MakeAllLights()
{
	int i;

	for (i = 0; i < cur_total; i++)
	{
		if (m_static[i].bActive == 1)
		{
			if (m_static[i].type == 0)
			{
				if (m_static[i].model_index == 3)
				{
					

					//m_Resources->m_Lights->Addpoint(XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f), XMFLOAT3(m_static[i].x, m_static[i].y + 5.0f, m_static[i].z), 17.0f,10.0f);
				}
			}
		}
	}
}

Concurrency::task<void> Statics::Update(float timeDelta, float timeTotal)
{

	if (bMakePhysicsWhenLoaded == true && bLoadingComplete==true)
	{
		
		bMakePhysicsWhenLoaded = false;
		MakePhysics();
		
	}


	return concurrency::create_task([this, timeDelta, timeTotal]
	{
		if (bLoadingComplete == true)
		{

			m_Fire->Reset();

			unsigned int seed = 237;
			float pnscale = m_Resources->m_LevelInfo.wind.w*1.04f;
			float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
			PerlinNoise pn(seed);

			int i;
			for (i = 0; i < total_static_models; i++)
			{
				//m_static_model[i]->model->Update(&m_Resources->m_Camera->GetEye());
			}

			for (i = 0; i < cur_total; i++)
			{
				if (m_static[i].bActive == 1)
				{

					if (m_static[i].model_index == 3 || m_static[i].model_index == 4)
					{
						m_static[i].dist = m_Resources->m_Camera->CheckPoint(m_static[i].x, m_static[i].y, m_static[i].z, m_static_model[m_static[i].model_index]->light_radius);
					}
					else
					{
						m_static[i].dist = m_Resources->m_Camera->CheckPoint(m_static[i].x, m_static[i].y, m_static[i].z, m_static_model[m_static[i].model_index]->m_meshModels[0]->Extents().Radius);
					}
					// spinning fruit
					if (m_static[i].type == 1)
					{
						if (m_Resources->m_Camera->DistanceEst(
							m_Resources->player_x, m_Resources->player_y, m_Resources->player_z,
							m_static[i].x, m_static[i].y, m_static[i].z) < 1.0f)
						{
							m_static[i].bActive = 0;
							m_Steam->CreateOne(m_static[i].x, m_static[i].y, m_static[i].z, 0.0f, 0.1f, 0.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 2);

							m_Resources->m_audio.PlaySoundEffect(SFXPop);
						}


						m_static[i].rot_y += 1.5f * timeDelta;
						if (m_static[i].rot_y > M_PI*2.0f)
						{
							m_static[i].rot_y -= M_PI*2.0f;
						}
						m_static[i].MakeMatrix();
					}


				}

			}

			current_index = 0;
			p_index.clear();
			p_index_car.clear();
			part_index pi;
			for (i = 0; i < cur_total; i++)
			{
				if (m_static[i].bActive == 1)
				{
					if (m_static[i].dist > 0.0f)
					{
						pi.dist = m_static[i].dist;
						pi.part_no = i;

						p_index.push_back(pi);

						pi.dist = m_static[i].dist;

						p_index_car.push_back(pi);

						//p_index[current_index].dist = m_static[i].dist;
						//p_index[current_index].part_no = i;

						//p_index_car[current_index].dist = m_static[i].dist;
						//p_index_car[current_index].part_no = i;

						//p_index[current_index].
						current_index++;
					}
				}
				else
				{

				}
			}

			std::sort(p_index.begin(), p_index.end(), statics_compare{});
			std::sort(p_index_car.begin(), p_index_car.end(), statics_car_compare{});

			//qsort(p_index_car, current_index, sizeof(part_index), StaticsSortCarCB);
			//qsort(p_index, current_index, sizeof(part_index), StaticsSortCB);




			for (int i = 0; i < 17; i++)
			{
				if (m_static_model[i]->bHasAnim == true)
				{
					m_skinnedMeshRenderer.UpdateAnimation(timeDelta, m_static_model[i]->m_meshModels, L"my_avatar|run");
				}

				// Initialize animated models.
				//for (Mesh* m : m_static_model[i]->m_meshModels)
				//{
				//
				//}

			}


			for (i = 0; i < current_index; i++)
			{

				static_t static_item = m_static[p_index[i].part_no];

				if (static_item.type == 0)
				{
					if (static_item.model_index == 3)
					{
						float light_dist = m_Resources->m_Camera->CheckPoint(static_item.x, static_item.y, static_item.z, m_static_model[static_item.model_index]->light_radius);
						if (true)//(light_dist > 0.0f)// && light_dist<17.0f)
						{
							float add_val = 0.0f;// (pn.noise(timeTotal + static_item.x*pnscale, timeTotal + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;
							float add_val2 = 0.0f;// (pn.noise(timeTotal + 2.0f + static_item.x*pnscale, timeTotal + 2.0f + static_item.z*pnscale, timeTotal*2.0f)*0.1f) - 0.05f;

							CG_POINT_LIGHT fire_point_light;
							fire_point_light.ambient = XMFLOAT4(static_item.colour.x, static_item.colour.y, static_item.colour.z, 0.5f);  ;
							fire_point_light.diffuse = static_item.colour; //XMFLOAT4(0.95f, 0.65f, 0.4f, 1.0f);  // static_item.colour;
							fire_point_light.specular = XMFLOAT4(static_item.colour.x, static_item.colour.y, static_item.colour.z, 0.5f);  ;
							fire_point_light.pos = XMFLOAT3(static_item.x+ add_val, static_item.y + 5.0f, static_item.z + add_val2);
							//fire_point_light.radius = m_static_model[static_item.model_index]->light_radius;
							fire_point_light._specular_power = 0.0f;
							fire_point_light.bCastShadows = true;
							m_Resources->m_Lights->AddPoint(fire_point_light);
							//m_Resources->m_Lights->Addpoint(XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f), XMFLOAT3(m_static[i].x, m_static[i].y + 5.0f, m_static[i].z), 17.0f, 25.0f);
							m_Fire->SetFire(static_item.x, static_item.y + 4.4f, static_item.z, 0.3f, timeTotal, timeDelta, fire_point_light.diffuse);

							//float randx = (((rand() % 10)*0.03f) - 0.05f);
							//float randy = (((rand() % 10)*0.03f));
							//float randz = (((rand() % 10)*0.03f) - 0.05f);
							//m_Scene->m_steam->CreateOne(ac_pos.getX() + randx, ac_pos.getY() + randy, ac_pos.getZ() + randz, pt.m_lateralFrictionDir1.getX(), pt.m_lateralFrictionDir1.getY(), pt.m_lateralFrictionDir1.getZ(), XMFLOAT4(m_Resources->m_LevelInfo.dust_col.x, m_Resources->m_LevelInfo.dust_col.y, m_Resources->m_LevelInfo.dust_col.z, 0.5f), 1.0f);

						}

					}

					if (static_item.model_index == 4)
					{
						float light_dist = m_Resources->m_Camera->CheckPoint(static_item.x, static_item.y, static_item.z, m_static_model[static_item.model_index]->light_radius);
						if (light_dist > 0.0f)// && light_dist<17.0f)
						{

							float add_val = pn.noise(timeTotal + static_item.x*pnscale, timeTotal + static_item.z*pnscale, timeTotal*8.0f)*0.3f;

							CG_POINT_LIGHT fire_point_light;
							fire_point_light.ambient = XMFLOAT4(0.2f+add_val, 0.1f+add_val, 0.1f+add_val, 1.0f);
							fire_point_light.diffuse = XMFLOAT4(0.4f+add_val, 0.3f+add_val, 0.3f+add_val, 1.0f);
							fire_point_light.specular = XMFLOAT4(0.2f+add_val, 0.1f+add_val, 0.1f+add_val, 0.9998f);
							fire_point_light.pos = XMFLOAT3(static_item.x, static_item.y + 0.5f, static_item.z);
							//fire_point_light.radius = m_static_model[static_item.model_index]->light_radius;
							fire_point_light._specular_power = 0.0f;
							fire_point_light.bCastShadows = false;
							m_Fire->SetFire(static_item.x, static_item.y, static_item.z, 0.8f, timeTotal, timeDelta, fire_point_light.diffuse);
							m_Resources->m_Lights->AddPoint(fire_point_light);
							//m_Resources->m_Lights->Addpoint(XMFLOAT4(0.2f*add_val, 0.1f*add_val, 0.1f*add_val, 1.0f), XMFLOAT4(0.5f*add_val, 0.2f*add_val, 0.2f*add_val, 1.0f), XMFLOAT4(0.3f*add_val, 0.1f*add_val, 0.1f*add_val, 0.9998f), XMFLOAT3(m_static[i].x, m_static[i].y + 0.5f, m_static[i].z), /*(add_val*5.0f)+10.0f*/15.0f, 0.0f);
						}
					}
				}


			}



			for (i = 0; i < cur_phy; i++)
			{
				//if (true)//m_Motion[i]->collision_range == 0)
				//{
				//	m_Motion[i]->MakeMatrix();
				//}

				float radius = m_static_model[m_static[m_Motion[i]->static_index].model_index]->m_meshModels[0]->Extents().Radius;
				m_Motion[i]->dist = m_Resources->m_Camera->CheckPoint(m_Motion[i]->m_Matrix._14, m_Motion[i]->m_Matrix._24, m_Motion[i]->m_Matrix._34, radius, &m_Motion[i]->cam_dist);

				if (m_Motion[i]->dist > 0.0f)
				{
					if (m_Motion[i]->cam_dist < 25.0f)///m_Resources->m_Camera->far_dist)
					{
						m_Motion[i]->m_rigidbody->setCollisionFlags(!btCollisionObject::CF_NO_CONTACT_RESPONSE);
					}
					else
					{
						m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
					}
				}
				else
				{
					m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
				}

			}



		}
	});

	
}


bool Statics::LoadBinary(int level)
{
	int i;
	ZeroMemory(m_static, sizeof(static_t)*MAX_STATICS);

	
	Reset();
	//return true;

	char info_filename[140];

	if (m_Resources->bContentFolder == false)
	{
		sprintf_s(info_filename, "%s\\LevelBinary\\Statics%d.bmp", m_Resources->local_file_folder, level);
	}
	else
	{
		sprintf_s(info_filename, "Assets\\LevelBinary\\Statics%d.bmp", level);
	}

	FILE * pFile;

	fopen_s(&pFile,info_filename, "rb");
	if (pFile != NULL)
	{
		fread(&cur_total, sizeof(int), 1, pFile);
		for (i = 0; i < cur_total; i++)
		{
			fread(&m_static[i], sizeof(static_t), 1, pFile);
			if (m_static[i].colour.x == 0.0f &&
				m_static[i].colour.y == 0.0f &&
				m_static[i].colour.z == 0.0f &&
				m_static[i].colour.w == 0.0f)
			{
				m_static[i].colour = XMFLOAT4(float(rand() % 100)*0.01f, float(rand() % 100)*0.01f, float(rand() % 100)*0.01f, 1.0f);
			}
		}
		fclose(pFile);

		return true;
	}
	else
	{

		cur_total = 0;
		SaveBinary(level);
		return false;
	}

}

bool Statics::SaveBinary(int level)
{
	int i;
	char info_filename[140];;

	sprintf_s(info_filename, "%s\\LevelBinary\\Statics%d.bmp", m_Resources->local_file_folder, level);

	FILE * pFile;

	fopen_s(&pFile,info_filename, "wb");
	if (pFile != NULL)
	{
		fwrite(&cur_total, sizeof(int), 1, pFile);
		for (i = 0; i < cur_total; i++)
		{
			fwrite(&m_static[i], sizeof(static_t), 1, pFile);
		}
		fclose(pFile);

	}
	else
	{
		return false;
	}

	return true;
}

void Statics::FindCenters()
{
	for (int i = 0; i < cur_total; i++)
	{

	}
}

void Statics::RenderDepth(int alpha_mode,int point_plane)
{
	int i;
	bool bRender;

	if (bLoadingComplete == false)
		return;

	//current_index = current_index / 2;
	for (i = 0; i < current_index; i++)
	{
		//if (alpha_mode == 0)
		//{
		p_index[i] = p_index_car[i];
		//}

		if (m_Resources->m_Lights->PointCubeWithin3DManhattanDistance(m_static[p_index[i].part_no].x,
			m_static[p_index[i].part_no].y,
			m_static[p_index[i].part_no].z,
			m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius + LIGHT_RADIUS) < 0.0f)
			continue;


		//FBXModelLoader* pModel = m_static_model[m_static[p_index[i].part_no].model_index]->model;
		bRender = true;
		if (point_plane > -1)
		{

			XMVECTOR rec_res = XMVector3Rotate(XMLoadFloat3(
				&XMFLOAT3(m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().CenterX,
					m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().CenterY,
					m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().CenterZ)), XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_static[p_index[i].part_no].model_matrix)));

			XMFLOAT3 vec_ref_float;
			XMStoreFloat3(&vec_ref_float, rec_res);
			//vec_ref_float = XMFLOAT3(0.0f,0.0f,0.0f);

			// check point light frustum
			bRender = m_Resources->m_Lights->CheckPointPlanes(point_plane, m_static[p_index[i].part_no].x + vec_ref_float.x, m_static[p_index[i].part_no].y + vec_ref_float.y, m_static[p_index[i].part_no].z + vec_ref_float.z, m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius);

		}

		if (bRender==true)//m_Resources->m_Lights->CheckPoint(m_static[p_index[i].part_no].x, m_static[p_index[i].part_no].y, m_static[p_index[i].part_no].z, m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius, nullptr) > 0.0f)
		{
			m_Resources->SetConstantModelBuffer(&m_static[p_index[i].part_no].model_matrix);

			m_Resources->m_Camera->UpdateConstantBuffer();

			for (Mesh* m : m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels)
			{
				if (m->alpha_mode == alpha_mode)
				{
					m->Render(*m_Resources);
				}
			}
		}
		else
		{
			//m_Resources->m_Camera->UpdateConstantBuffer();
		}


		//m_Resources->se
		/*
		if (m_static[p_index[i].part_no].bHasFlag == true && alpha_mode == 0)
		{
			//m_Resources->SetCull(false);
			//m_Resources->EnableBlendingTexAlph();
			//m_static[p_index[i].part_no].model_matrix._14 += 0.1f;

			m_static[p_index[i].part_no].flag_pos += 0.025f;
			m_Flag->UpdateGeneratedTerrain(0, m_static[p_index[i].part_no].flag_pos, m_static[p_index[i].part_no].model_matrix);
			m_Flag->Render(m_static[p_index[i].part_no].x, m_static[p_index[i].part_no].y + 7.0f, m_static[p_index[i].part_no].z);
			//m_Resources->SetCull(true);
		}
		*/
	}

}


void Statics::Render(int alpha_mode,bool _bdef)
{
	int i;

	if (bLoadingComplete == false)
		return;

	//current_index = current_index / 2;
	for (i = 0; i < current_index; i++)
	{
		if (alpha_mode == 0)
		{
			p_index[i] = p_index_car[i];
		}

		//FBXModelLoader* pModel = m_static_model[m_static[p_index[i].part_no].model_index]->model;

		m_Resources->SetConstantModelBuffer(&m_static[p_index[i].part_no].model_matrix);

		m_Resources->m_Camera->UpdateConstantBuffer();
		
		for (Mesh* m : m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels)
		{
			if (m->alpha_mode == alpha_mode)
			{
				
				if (m->Tag != nullptr)
				{
					//
					// mesh has animation
					//
					m_Resources->SetSkinShader();
					m_skinnedMeshRenderer.RenderSkinnedMesh(m, *m_Resources);
				}
				else
				{
					//
					// render as usual
					//
					if (alpha_mode == 2)
					{

					}
					else
					{
						if (_bdef == true)
						{
							m_Resources->SetDeferredShader();
						}
						else
						{
							m_Resources->SetShinyShader();
						}
					}

					m->Render(*m_Resources);
				}
				
				//m->Render(*m_Resources);
			}
		}



		//m_Resources->se
		//if (m_static[p_index[i].part_no].bHasFlag == true && alpha_mode==0)
		//{
		//	//m_Resources->SetCull(false);
		//	//m_Resources->EnableBlendingTexAlph();
		//	//m_static[p_index[i].part_no].model_matrix._14 += 0.1f;

		//	m_static[p_index[i].part_no].flag_pos += 0.025f;
		//	m_Flag->UpdateGeneratedTerrain(0, m_static[p_index[i].part_no].flag_pos, m_static[p_index[i].part_no].model_matrix);
		//	m_Flag->Render(m_static[p_index[i].part_no].x, m_static[p_index[i].part_no].y+7.0f, m_static[p_index[i].part_no].z);
		//	//m_Resources->SetCull(true);
		//}

	}
}

void Statics::SetCurrentSelectedindex(int _ind)
{
	current_selected_index = _ind;
}

void Statics::RenderPointer()
{
	int i;

	m_Resources->DisableBlending();

	for (Mesh* m : m_static_model[current_selected_index]->m_meshModels)
	{
		if (m->alpha_mode == 0)
		{
			if (m->Tag != nullptr)
			{
				//
				// mesh has animation
				//
				m_Resources->SetSkinShader();
				m_skinnedMeshRenderer.RenderSkinnedMesh(m, *m_Resources);
			}
			else
			{
				//
				// render as usual
			
				m_Resources->SetShinyShader();
				m->Render(*m_Resources);
			}
		}
	}

	//m_static_model[current_selected_index]->model->Render(0);
	m_Resources->EnableBlendingTexAlph();

	for (Mesh* m : m_static_model[current_selected_index]->m_meshModels)
	{
		if (m->alpha_mode == 1 || m->alpha_mode == 2)
		{
			m->Render(*m_Resources);
		}
	}

	//m_static_model[current_selected_index]->model->Render(1);
	m_Resources->DisableBlending();
}
