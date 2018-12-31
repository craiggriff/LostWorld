#include "pch.h"
#include "Stuff.h"

#include "DefPhysics.h"

using namespace Game;
using namespace DirectX;

#define MAX_STUFF 4000


// std::sort with inlining
struct stuff_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

// std::sort with inlining
struct stuff_car_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist > b.dist;
	}
};



int StuffSortCB(const VOID* arg1, const VOID* arg2)
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
			return -1;
		}
		else
		{
			return 1;
		}
	}
}

int StuffSortCarCB(const VOID* arg1, const VOID* arg2)
{
	part_index* p1 = (part_index*)arg1;
	part_index* p2 = (part_index*)arg2;

	if (p1->dist == p2->dist)
	{
		return 0;
	}
	else
	{
		if (p1->dist>p2->dist)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}

Stuff::Stuff(AllResources* p_Resources)
{
	int i;
	m_Resources = p_Resources;

	m_deviceResources = p_Resources->m_deviceResources;

	max_num = 200;

	m_stuff = new stuff_t[MAX_STUFF];


	m_Motion = new MeshMotion*[MAX_STUFF];

	
	for (i = 0; i < MAX_STUFF; i++)
	{
		m_Motion[i] = nullptr;
	}
	

	p_index.clear();
	p_index_car.clear();


	for (i = 0; i < 10; i++)
	{
		m_stuff_model[i] = new stuff_model_t;
	}

	cur_total = 0;

	bPhysical = false;

	cur_phy = 0;

	current_selected_index = 0;

	bLoadingComplete = false;

}


void Stuff::Reset()
{
	cur_total = 0;
	cur_phy = 0;

	current_selected_index = 0;

	for (int i = 0; i < MAX_STUFF; i++)
	{
		m_stuff[i].bActive = 0;

	}

}


void Stuff::SetPhysical(bool _bIsPhysical)
{
	//if (bLoadingComplete == false)
	//	return;

	if (_bIsPhysical == true && bPhysical == false)
	{
		cur_phy = 0;
		for (int i = 0; i < cur_total; i++)
		{
			if (m_stuff[i].bActive == 1)
			{
				if (true)//(m_Motion[cur_phy] == nullptr)
				{
					m_Motion[cur_phy] = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics);
				}
				m_stuff[i].info.group = m_stuff_model[m_stuff[i].model_index]->group;
				m_stuff[i].info.mask = m_stuff_model[m_stuff[i].model_index]->mask;
				m_stuff[i].info.mrf = m_stuff_model[m_stuff[i].model_index]->mrf;

				m_Motion[cur_phy]->ob_info = m_stuff[i].info;
				m_Motion[cur_phy]->collision_range = m_stuff_model[m_stuff[i].model_index]->collision_range;


				switch (m_stuff_model[m_stuff[i].model_index]->physical_shape)
				{
				case 0:m_Motion[cur_phy]->MakePhysicsBoxFromFBX(m_stuff_model[m_stuff[i].model_index]->m_meshModels[0], 1.0f); break;
				case 1:m_Motion[cur_phy]->MakePhysicsCylinderFBX(m_stuff_model[m_stuff[i].model_index]->m_meshModels[0], 1.0f); break;
				case 2:m_Motion[cur_phy]->MakePhysicsSphereFromFBX(m_stuff_model[m_stuff[i].model_index]->m_meshModels[0], 1.0f); break;
				case 3:m_Motion[cur_phy]->MakePhysicsEllipseoidFromFBX(m_stuff_model[m_stuff[i].model_index]->m_meshModels[0], 1.5f); break;
				case 4:m_Motion[cur_phy]->MakePhysicsConvexHullFromFBX(m_stuff_model[m_stuff[i].model_index]->m_meshModels[0], 1.0f); break;
				}

				if (m_Motion[cur_phy]->collision_range == 1) // 1 is static close objects
				{
					m_Motion[cur_phy]->MakeMatrix();
					//m_Motion[cur_phy]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
					m_Motion[cur_phy]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
				}

				cur_phy++;
			}
		}

		bPhysical = true;
	}
	else
	{
		if (bPhysical == true && _bIsPhysical == false)
		{
			bPhysical = false;
		}
		/*
		if (_bIsPhysical == false && bPhysical == true)
		{
			for (int i = 0; i < cur_phy; i++)
			{
				//
				m_Resources->m_Physics.m_dynamicsWorld->removeRigidBody(m_Motion[i]->m_rigidbody);
				delete m_Motion[i]->m_rigidbody->getMotionState();
				m_Motion[i] = nullptr;

			}
			bPhysical = false;
		}
		*/
	}
}


//void Stuff::Rotate

void Stuff::Clear(float x, float z)
{
	for (int i = 0; i < cur_total; i++)
	{
		if (m_stuff[i].bActive == 1 &&
			m_stuff[i].info.pos.x == x &&
			m_stuff[i].info.pos.z == z)
		{
			if (i == cur_total - 1)
			{
				cur_total--;
			}
			else
			{
				if (cur_total - 1 > 0)
				{
					memcpy(&m_stuff[i], &m_stuff[cur_total - 1], sizeof(static_t));
					//m_stuff[i].model_matrix = m_stuff[cur_total].model_matrix;
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

void Stuff::CreateOne(float x, float y, float z, float height_from_t, float _scale, float rx, float ry, float rz, bool _FromLastHeight)
{
	int i;
	bool bFound;
	
	XMFLOAT3 mrf;

	if (cur_total >= MAX_STUFF)
		return;

	stack_pointer_y = 0.0f;
	
	if (_FromLastHeight == true)
	{
		bFound = true;
	}
	else
	{
		index_found = -99;
		bFound = false;
		found_y = 0.0f;
		for (i = 0; i < cur_total; i++)
		{
			if (m_stuff[i].bActive == 1 &&
				m_stuff[i].info.pos.x == x &&
				m_stuff[i].info.pos.z == z)
			{
				if (m_stuff[i].info.pos.y>found_y)
				{
					found_y = m_stuff[i].info.pos.y;
					index_found = i;
					bFound = true;
				}
			}
		}
	}

	if (bFound == true)
	{
		height_from_t += (found_y - y)+ m_stuff_model[m_stuff[index_found].model_index]->m_meshModels[0]->Extents().MaxY + m_stuff_model[current_selected_index]->m_meshModels[0]->Extents().MaxY;
		y = found_y + m_stuff_model[m_stuff[index_found].model_index]->m_meshModels[0]->Extents().MaxY + m_stuff_model[current_selected_index]->m_meshModels[0]->Extents().MaxY;
	}
	
	switch (current_selected_index)
	{
	case 0:
		mrf = XMFLOAT3(1.5f, 0.5f, 0.5f); break;
	}

	if (true)//(bFound == false)
	{
		m_stuff[cur_total].height_from_terrain = height_from_t;// +m_stuff_model[current_selected_index]->box_extents.y;

		m_stuff[cur_total].info.pos = XMFLOAT3(x, y, z);
		m_stuff[cur_total].info.dir = XMFLOAT3(ry, rx, rz);
		m_stuff[cur_total].info.mrf = XMFLOAT3(ry, rx, rz); //m_stuff_model[current_selected_index]->mrf;
		m_stuff[cur_total].info.group = m_stuff_model[current_selected_index]->group;// (COL_CARBODY | COL_WHEEL | COL_TERRAIN);
		m_stuff[cur_total].info.mask = m_stuff_model[current_selected_index]->mask;//(COL_OBJECTS);

		m_stuff[cur_total].model_index = current_selected_index;
		m_stuff[cur_total].model_matrix = m_Resources->MakeMatrix(x, y, z, rx, ry, rz, 1.0f);
		m_stuff[cur_total].scale = _scale;
		m_stuff[cur_total].bActive = 1;

		cur_total++;
	}
}

//concurrency::task<void> LoadModel(wchar_t* filename,stuff_model_t* p_model,)

void Stuff::LoadStuff(bool _setPhysical)
{

	std::vector<concurrency::task<void>> tasks;

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"mariobox.cmo",
		L"",
		L"",
		m_stuff_model[0]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[0]->m_meshModels[0]->SetMaterialTexture(L"yel", 0, m_Resources->m_Textures->LoadTexture("yell"), 0);
		m_stuff_model[0]->m_meshModels[0]->SetMaterialTexture(L"bla", 0, m_Resources->m_Textures->LoadTexture("gemred"), 0);
		m_stuff_model[0]->m_meshModels[0]->SetMaterialTexture(L"grn", 0, m_Resources->m_Textures->LoadTexture("gemred"), 0);

		m_stuff_model[0]->mrf = XMFLOAT3(0.3f, 0.5f, 0.5f);
		m_stuff_model[0]->physical_shape = SHAPE_BOX;
		m_stuff_model[0]->collision_range = 0;
		m_stuff_model[0]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[0]->mask = (COL_OBJECTS | COL_RAY);

		
	}));


	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"oil_barrel.cmo",
		L"",
		L"",
		m_stuff_model[1]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[1]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("barrel"), 0);

		m_stuff_model[1]->mrf = XMFLOAT3(0.8f, 0.5f, 0.5f);
		m_stuff_model[1]->physical_shape = SHAPE_CYLINDER;
		m_stuff_model[1]->collision_range = 0;
		m_stuff_model[1]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[1]->mask = (COL_OBJECTS | COL_RAY);
	}));


	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"beachball.cmo",
		L"",
		L"",
		m_stuff_model[2]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[2]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("beachball"), 0);

		//m_stuff_model[0]->m_meshModels[1]->SetMaterialTexture(L"mat1_leaves", 0, m_Resources->m_Textures->LoadTexture("tree_leaves"), 1);
		//m_stuff_model[2]->m_meshModels[0]->SetMaterialNormal(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("fireplace_uv"), 2.0f);
		m_stuff_model[2]->mrf = XMFLOAT3(0.1f, 0.9f, 0.5f);
		m_stuff_model[2]->physical_shape = SHAPE_SPHERE;
		m_stuff_model[2]->collision_range = 0;
		m_stuff_model[2]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[2]->mask = (COL_OBJECTS | COL_RAY);

		//m_stuff_model[2]->m_meshModels[0]->SetMaterialNormal(L"mat1_trunk", 0, m_Resources->m_Textures->LoadTexture("178_norm"), 1.0f);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"column.cmo",
		L"",
		L"",
		m_stuff_model[3]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[3]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("Granite"), 0);
		m_stuff_model[3]->m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("bigrocks_norm"), 0.5f);

		m_stuff_model[3]->mrf = XMFLOAT3(2.1f, 0.1f, 0.5f);
		m_stuff_model[3]->physical_shape = SHAPE_BOX;
		m_stuff_model[3]->collision_range = 0;
		m_stuff_model[3]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[3]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"block.cmo",
		L"",
		L"",
		m_stuff_model[4]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[4]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("Granite"), 0);
		m_stuff_model[4]->mrf = XMFLOAT3(1.0f, 0.1f, 1.0f);
		m_stuff_model[4]->physical_shape = SHAPE_BOX;
		m_stuff_model[4]->collision_range = 0;
		m_stuff_model[4]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[4]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"block_small.cmo",
		L"",
		L"",
		m_stuff_model[5]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[5]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("woodfloor"), 0);

		m_stuff_model[5]->mrf = XMFLOAT3(0.0f, 0.1f, 0.5f);
		m_stuff_model[5]->physical_shape = SHAPE_BOX;
		m_stuff_model[5]->collision_range = 0;
		m_stuff_model[5]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[5]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"big_disk.cmo",
		L"",
		L"",
		m_stuff_model[6]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[6]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("woodfloor"), 0);

		m_stuff_model[6]->mrf = XMFLOAT3(1.0f, 0.1f, 0.5f);
		m_stuff_model[6]->physical_shape = SHAPE_CYLINDER;
		m_stuff_model[6]->collision_range = 0;
		m_stuff_model[6]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[6]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"pumpkined.cmo",
		L"",
		L"",
		m_stuff_model[7]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[7]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("pumpkintex"), 0);

		m_stuff_model[7]->mrf = XMFLOAT3(0.2f, 0.5f, 1.5f);
		m_stuff_model[7]->physical_shape = SHAPE_ELLIPSEOID;
		m_stuff_model[7]->collision_range = 0;
		m_stuff_model[7]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[7]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"fence1.cmo",
		L"",
		L"",
		m_stuff_model[8]->m_meshModels,
		1.0f).then([this]()
	{
		m_stuff_model[8]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("fence1"), 0);

		m_stuff_model[8]->mrf = XMFLOAT3(0.0f, 0.1f, 0.5f);
		m_stuff_model[8]->physical_shape = SHAPE_BOX;
		m_stuff_model[8]->collision_range = 1;
		m_stuff_model[8]->group = (COL_CARBODY | COL_WHEEL | COL_OBJECTS | COL_CHAR);
		m_stuff_model[8]->mask = (COL_OBJECTS | COL_RAY);
	}));

	tasks.push_back(Mesh::LoadFromFileAsync(
		*m_Resources,
		L"pin.cmo",
		L"",
		L"",
		m_stuff_model[9]->m_meshModels,
		0.5f).then([this]()
	{
		m_stuff_model[9]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("pin"), 0);

		m_stuff_model[9]->mrf = XMFLOAT3(0.2f, 0.5f, 0.5f);
		m_stuff_model[9]->physical_shape = SHAPE_CYLINDER;
		m_stuff_model[9]->collision_range = 0;
		m_stuff_model[9]->group = (COL_CARBODY | COL_WHEEL | COL_TERRAIN | COL_OBJECTS | COL_CHAR);
		m_stuff_model[9]->mask = (COL_OBJECTS | COL_RAY);
	}));

	total_stuff_models = 11;

	// make the physics
	when_all(begin(tasks), end(tasks)).then([this, _setPhysical]
	{

		SetPhysical(_setPhysical);
		bLoadingComplete = true;
	});;

	


}



bool Stuff::LoadBinary(int level)
{
	int i;

	current_index = 0;
	cur_phy = 0;
	bPhysical = false;

	char info_filename[140];

	if (m_Resources->bContentFolder == false)
	{
		sprintf_s(info_filename, "%s\\LevelBinary\\Stuff%d.bmp", m_Resources->local_file_folder, level);
	}
	else
	{
		sprintf_s(info_filename, "Assets\\LevelBinary\\Stuff%d.bmp", level);
	}

	FILE * pFile;

	fopen_s(&pFile,info_filename, "rb");
	if (pFile != NULL)
	{
		fread(&cur_total, sizeof(int), 1, pFile);
		for (i = 0; i < MAX_STUFF; i++)
		{
			fread(&m_stuff[i], sizeof(stuff_t), 1, pFile);
		}
		fclose(pFile);

		//return true;
	}
	else
	{
		for (i = 0; i < MAX_STUFF; i++)
		{
			m_stuff[i].bActive = 0;
		}
		cur_total = 0;
		//return false;
	}

	if (m_Resources->bDevelopment == false)
	{
		LoadStuff(true);
		//SetPhysical(true);
	}
	else
	{
		LoadStuff(false);
	}
	return true;
}

bool Stuff::SaveBinary(int level)
{
	int i;
	char info_filename[140];;

	sprintf_s(info_filename, "%s\\LevelBinary\\Stuff%d.bmp", m_Resources->local_file_folder, level);

	FILE * pFile;

	fopen_s(&pFile,info_filename, "wb");
	if (pFile != NULL)
	{
		fwrite(&cur_total, sizeof(int), 1, pFile);
		for (i = 0; i < MAX_STUFF; i++)
		{
			fwrite(&m_stuff[i], sizeof(stuff_t), 1, pFile);
		}
		fclose(pFile);

	}
	else
	{
		return false;
	}

	return true;
}


Concurrency::task<void> Stuff::Update()
{
	return Concurrency::create_task([this]
	{
		if (bLoadingComplete == true)
		{

			int i;
			current_index = 0;
			/*
			for (i = 0; i < total_stuff_models; i++)
			{
				m_stuff_model[i]->model->Update(&m_Resources->m_Camera->GetEye());
			}
			*/
			if (bPhysical == true)
			{
				p_index.clear();
				p_index_car.clear();

				part_index pi;
				for (i = 0; i < cur_phy; i++)
				{
					if (true)//m_Motion[i]->collision_range == 0)
					{
						m_Motion[i]->MakeMatrix();
					}

					m_Motion[i]->dist = m_Resources->m_Camera->CheckPoint(m_Motion[i]->m_Matrix._14, m_Motion[i]->m_Matrix._24, m_Motion[i]->m_Matrix._34, m_Motion[i]->m_model->Extents().Radius, &m_Motion[i]->cam_dist);

					if (m_Motion[i]->dist > 0.0f)
					{
						pi.dist = m_Motion[i]->dist;
						pi.part_no = i;

						p_index.push_back(pi);
						//p_index[current_index].dist = m_Motion[i]->dist;
						//p_index[current_index].part_no = i;
						//p_index[current_index].
						current_index++;
					}

					if (m_Motion[i]->collision_range == 1)
					{

						if (m_Motion[i]->cam_dist < 25.0f) // filter out far from camera
						{
							m_Motion[i]->m_rigidbody->setCollisionFlags(!btCollisionObject::CF_NO_CONTACT_RESPONSE);
							//m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
						}
						else
						{
							m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
						}

					}
					else
					{
						if (m_Motion[i]->cam_dist < m_Resources->m_Camera->far_dist)
						{
							//m_Motion[i]->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
							m_Motion[i]->m_rigidbody->setCollisionFlags(!btCollisionObject::CF_STATIC_OBJECT | !btCollisionObject::CF_NO_CONTACT_RESPONSE);
							//m_Motion[i]->m_rigidbody->setCollisionFlags();

						}
						else
						{
							m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
							//m_Motion[i]->m_rigidbody->setCollisionFlags();
							//m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_);

							/*
							if (m_Motion[i]->m_rigidbody->getActivationState() == ISLAND_SLEEPING)
							{
							//m_Motion[i]->m_rigidbody->setActivationState(DISABLE_SIMULATION);
							m_Motion[i]->m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
							}
							*/
						}
					}
				}

				std::sort(p_index.begin(), p_index.end(), stuff_compare{});

				//qsort(p_index, current_index, sizeof(part_index), StuffSortCB);
			}
		}
	});

}


void Stuff::RenderDepth(int alpha_mode, int point_plane)
{
	int i;
	bool bRender;

	//current_index = 0;
	if (bLoadingComplete == false)
		return;

	if (bPhysical == true)
	{
		for (i = 0; i < current_index; i++)
		{



			if (m_Resources->m_Lights->PointCubeWithin3DManhattanDistance(m_Motion[p_index[i].part_no]->m_Matrix._14,
				m_Motion[p_index[i].part_no]->m_Matrix._24,
				m_Motion[p_index[i].part_no]->m_Matrix._34,
				m_Motion[p_index[i].part_no]->m_model->Extents().Radius + LIGHT_RADIUS) < 0.0f)
				continue;



			bRender = true;
			if (point_plane > -1)
			{
				// check point light frustum
				bRender = m_Resources->m_Lights->CheckPointPlanes(point_plane, m_Motion[p_index[i].part_no]->m_Matrix._14, m_Motion[p_index[i].part_no]->m_Matrix._24, m_Motion[p_index[i].part_no]->m_Matrix._34, m_Motion[p_index[i].part_no]->m_model->Extents().Radius);

			}


			if (bRender == true && alpha_mode== m_Motion[p_index[i].part_no]->m_model->alpha_mode)//m_Resources->m_Lights->CheckPoint(m_Motion[p_index[i].part_no]->m_Matrix._14, m_Motion[p_index[i].part_no]->m_Matrix._24, m_Motion[p_index[i].part_no]->m_Matrix._34, m_Motion[p_index[i].part_no]->m_model->Extents().Radius, nullptr) > 0.0f)
			{

				m_Motion[p_index[i].part_no]->GetMatrix(m_Resources->ConstantModelBuffer());
				m_Resources->m_Camera->UpdateConstantBuffer();

				Mesh* pModel = m_Motion[p_index[i].part_no]->m_model;

				pModel->Render(*m_Resources);
			}
		}
	}
	else
	{
		current_index = 0;
		p_index.clear();
		p_index_car.clear();
		part_index pi;
		for (i = 0; i<cur_total; i++)
		{
			if (m_stuff[i].bActive == 1)
			{
				float dist = m_Resources->m_Camera->CheckPoint(m_stuff[i].info.pos.x, m_stuff[i].info.pos.y, m_stuff[i].info.pos.z, m_stuff_model[m_stuff[i].model_index]->m_meshModels[0]->Extents().Radius);
				if (dist>0.0f)
				{
					pi.dist = dist;
					pi.part_no = i;

					p_index.push_back(pi);
					//p_index[current_index].dist = dist;
					//p_index[current_index].part_no = i;
					current_index++;

				}
			}
			else
			{

			}
		}

		//qsort(p_index, current_index, sizeof(part_index), StuffSortCB);

		std::sort(p_index.begin(), p_index.end(), stuff_compare{});

		for (i = 0; i < current_index; i++)
		{
			//FBXModelLoader* pModel = m_stuff_model[m_stuff[p_index[i].part_no].model_index]->model;


			bRender = true;
			if (point_plane > -1)
			{
				// check point light frustum
				bRender = m_Resources->m_Lights->CheckPointPlanes(point_plane, m_stuff[i].info.pos.x, m_stuff[i].info.pos.y, m_stuff[i].info.pos.z, m_stuff_model[m_stuff[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius);

			}


			if (bRender == true && alpha_mode == m_stuff_model[m_stuff[p_index[i].part_no].model_index]->m_meshModels[0]->alpha_mode)//m_Resources->m_Lights->CheckPoint(m_Motion[p_index[i].part_no]->m_Matrix._14, m_Motion[p_index[i].part_no]->m_Matrix._24, m_Motion[p_index[i].part_no]->m_Matrix._34, m_Motion[p_index[i].part_no]->m_model->Extents().Radius, nullptr) > 0.0f)
			{

				m_Resources->SetConstantModelBuffer(&m_stuff[p_index[i].part_no].model_matrix);

				m_Resources->m_Camera->UpdateConstantBuffer();


				Mesh* pModel = m_stuff_model[m_stuff[p_index[i].part_no].model_index]->m_meshModels[0];

				if (pModel->alpha_mode == alpha_mode)
				{
					pModel->Render(*m_Resources);
				}
			}
		}
	}
}

void Stuff::Render(int alpha_mode)
{
	int i;
	//current_index = 0;
	if (bLoadingComplete == false)
		return;

	if (bPhysical == true)
	{
		
		/*
		for (i = 0; i<cur_phy; i++)
		{
			bool bRender = true; // set to false to see range
			*/


		for (i = 0; i < current_index; i++)
		{
			m_Motion[p_index[i].part_no]->GetMatrix(m_Resources->ConstantModelBuffer());
			m_Resources->m_Camera->UpdateConstantBuffer();

			Mesh* pModel = m_Motion[p_index[i].part_no]->m_model;

			if (pModel->alpha_mode == alpha_mode)
			{
				 pModel->Render(*m_Resources);
			}
			
		}
	}
	else
	{
		current_index = 0;
		p_index.clear();
		p_index_car.clear();
		part_index pi;
		for (i = 0; i<cur_total; i++)
		{
			if (m_stuff[i].bActive == 1)
			{
				float dist = m_Resources->m_Camera->CheckPoint(m_stuff[i].info.pos.x, m_stuff[i].info.pos.y, m_stuff[i].info.pos.z, m_stuff_model[m_stuff[i].model_index]->m_meshModels[0]->Extents().Radius);				
				if(dist>0.0f)
				{
					pi.dist = dist;
					pi.part_no = i;

					p_index.push_back(pi);

					//p_index[current_index].dist = dist;
					//p_index[current_index].part_no = i;
					current_index++;
				}
			}
			else
			{

			}
		}
		
		std::sort(p_index.begin(), p_index.end(), stuff_compare{});
		//qsort(p_index, current_index, sizeof(part_index), StuffSortCB);
		
		for (i = 0; i < current_index; i++)
		{
			//FBXModelLoader* pModel = m_stuff_model[m_stuff[p_index[i].part_no].model_index]->model;

			m_Resources->SetConstantModelBuffer(&m_stuff[p_index[i].part_no].model_matrix);

			m_Resources->m_Camera->UpdateConstantBuffer();


			Mesh* pModel = m_stuff_model[m_stuff[p_index[i].part_no].model_index]->m_meshModels[0];

			if (pModel->alpha_mode == alpha_mode)
			{
				pModel->Render(*m_Resources);
			}

		}
	}
}

void Stuff::SetCurrentSelectedindex(int _ind)
{
	current_selected_index = _ind;
}

void Stuff::RenderPointer(DirectX::XMFLOAT4X4* t_matrix, bool _FromLastHeight)
{
	int i;
	bool bFound;

	//index_found = -99;
	//bFound = false;
	//found_y=0.0f;


	stack_pointer_y = 0.0f;

	if (_FromLastHeight == true)
	{
		bFound = true;
	}
	else
	{
		index_found = -99;
		bFound = false;
		found_y = 0.0f;
		for (i = 0; i < cur_total; i++)
		{
			if (m_stuff[i].bActive == 1 &&
				m_stuff[i].info.pos.x == t_matrix->_14 &&
				m_stuff[i].info.pos.z == t_matrix->_34)
			{
				if (m_stuff[i].info.pos.y>found_y)
				{
					found_y = m_stuff[i].info.pos.y;
					index_found = i;
					bFound = true;
				}
				///break;
			}
		}
	}

	if (bFound == true && index_found!=-99)
	{
		m_Resources->m_Camera->m_constantBufferData.model._24 = 
			found_y + 
			m_stuff_model[m_stuff[index_found].model_index]->m_meshModels[0]->Extents().MaxY +
			m_stuff_model[current_selected_index]->m_meshModels[0]->Extents().MaxY;

		stack_pointer_y = found_y +
			m_stuff_model[m_stuff[index_found].model_index]->m_meshModels[0]->Extents().MaxY;

		m_Resources->m_Camera->UpdateConstantBuffer();
		//exit(1);
	}


	m_stuff_model[current_selected_index]->m_meshModels[0]->Render(*m_Resources);

}
