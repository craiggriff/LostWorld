#include "pch.h"
#include "Fog.h"
#include "DefParticle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;



// std::sort with inlining
struct fog_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist > b.dist;
	}
};





void Fog::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_FOG_PARTICLES;

	snow = new fog_t[m_maxParticles];

	//p_index = new part_index[m_maxParticles]; 
	p_index.clear();

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	

	current_index = 0;

	current_point = 0;

	total_collected = 0;

	rand_state = 0;  // <- state of the RNG (input and output)

	InitializeBuffers(_bInstanced);

	m_indexCount = 0;

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"Assets/Textures/snow.dds", nullptr, &m_Texture, MAXSIZE_T);

	m_Texture = m_Resources->m_Textures->LoadTexture("misc_smoke");

	for (int i = 0; i<m_maxParticles; i++)
	{
		snow[i].bActive = 0;
	}

}


void Fog::CreateOne(float x, float y, float z)
{
	//int point_find = 0;
	//point_find = current_point;
	if (snow[current_point].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
		snow[current_point].bActive = 1;
		snow[current_point].vel_y = -0.1f;//  (((rand() % 100) / 50.0f) + 1.0f)*0.1f;
		snow[current_point].vel_x = (((rand() % 100) / 50.0f) + 1.0f)*0.2f; // 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
		snow[current_point].vel_z = (((rand() % 100) / 50.0f) + 1.0f)*0.2f;
		snow[current_point].x = x;
		snow[current_point].y = y;
		snow[current_point].z = z;
		snow[current_point].v1 = 0.125f*(float)(rand() % 6);
		snow[current_point].v2 = snow[current_point].v1 + 0.0625f;
		snow[current_point].life = 10.0f;
		snow[current_point].stopped = 0;
		snow[current_point].angle = 0.0f;
		snow[current_point].angle_mom = (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

		current_point++;
		//break;
	}
	if (current_point>m_maxParticles - 2)
		current_point = 0;

	/*
	do
	{
	if (point_find>m_maxParticles - 2)
	point_find = 0;

	if (snow[point_find].bActive == 0) //&& z < p_Level->front_back_walls && z > -p_Level->front_back_walls)
	{
	snow[point_find].bActive = 1;
	snow[point_find].vel_y = (((rand() % 100) / 50.0f) + 1.0f)*0.2f;
	snow[point_find].vel_x = 0.0f; //((localrand() % 50) / 150.0f)*0.2f;
	snow[point_find].x = x;
	snow[point_find].y = y;
	snow[point_find].z = z;
	snow[point_find].v1 = 0.125f*(float)(rand() % 6);
	snow[point_find].v2 = snow[point_find].v1 + 0.0625f;
	snow[point_find].life = 10.0f;
	snow[point_find].stopped = 0;
	snow[point_find].angle = 0.0f;
	snow[point_find].angle_mom = (((rand() % 50) / 50.0f) - 0.5f)*0.1f;

	point_find++;
	break;
	}
	else
	{
	point_find++;
	}

	} while (point_find < m_maxParticles - 2 && point_find!=current_point);

	current_point = point_find;
	if (current_point>m_maxParticles - 2)
	current_point = 0;
	*/

}

unsigned Fog::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}



void Fog::Reset()
{
	for (int i = 0; i<m_maxParticles; i++)
	{
		snow[i].bActive = 0;
	}
	for (int i = 0; i < 120; i++)
	{
		Update(0.0f, 1.1f);
	}
}



Fog::~Fog(void)
{


}



Concurrency::task<void> Fog::Update(float timeTotal, float timeDelta)
{
	return Concurrency::create_task([this, timeDelta, timeTotal]
	{
		int i;


		XMFLOAT3 cam_pos = m_Resources->m_Camera->GetEye();

		cam_pos.x += m_Resources->m_Camera->LookingX()*15.0f;
		cam_pos.z += m_Resources->m_Camera->LookingZ()*15.0f;


		float change, x_movement, z_movement;

		x_movement = (timeTotal*m_Resources->m_LevelInfo.wind.x)*0.2f;
		z_movement = (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.2f;

		change = 0.0f;

		float pn_scale = 0.2f;

		/*
		m_Resources->m_uiControl->SetInfoVal(0, m_Resources->m_Camera->PositionX());
		m_Resources->m_uiControl->SetInfoVal(1, cam_pos.y);
		m_Resources->m_uiControl->SetInfoVal(2, cam_pos.z);

		m_Resources->m_uiControl->SetInfoVal(3, m_Resources->m_Camera->LookingX());
		m_Resources->m_uiControl->SetInfoVal(4, m_Resources->m_Camera->LookingY());
		m_Resources->m_uiControl->SetInfoVal(5, m_Resources->m_Camera->LookingZ());
		*/
		unsigned int seed = 237;
		float pnscale = m_Resources->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);


		for (i = 0; i < m_maxParticles; i++)
		{
			if (snow[i].bActive == 1)
			{

				//float diff_x = m_Resources->m_LevelInfo.wind.x*timeDelta*0.2f;
				//float diff_y = -snow[i].vel_y*timeDelta*8.0f;
				//float diff_z = m_Resources->m_LevelInfo.wind.z*timeDelta*0.2f;

				float speed = (pn.noise(((float)(snow[i].x*pn_scale) + x_movement)*m_Resources->m_LevelInfo.wind.w, ((float)(snow[i].z*pn_scale) + z_movement)*m_Resources->m_LevelInfo.wind.w, change))*m_Resources->m_LevelInfo.wind.y;
				snow[i].alpha = speed;
				//(pn.noise(((float)(j)+x_movement)*m_Resources->m_LevelInfo.wind.w, ((float)(i)+z_movement)*m_Resources->m_LevelInfo.wind.w, change))*m_Resources->m_LevelInfo.wind.y;

				//float speed = ((pn.noise((snow[i].x + diff_x)*pnscale, (snow[i].z + diff_z)*pnscale, (snow[i].z + diff_y)*pnscale)))*wave_height;
				//diff_x = diff_x * speed;
				//diff_z = diff_z * speed;




				snow[i].x += snow[i].vel_x*timeDelta;
				snow[i].y += snow[i].vel_y*timeDelta;
				snow[i].z += snow[i].vel_z*timeDelta;
				//snow[i].y += diff_y;
				//snow[i].x += diff_x;
				//snow[i].z += diff_z;

				//snow[i].y += -snow[i].vel_y;
				//snow[i].x += snow[i].vel_x;
				//snow[i].z += snow[i].vel_z;

				//if (snow[i].y <= 0.0f)
				//{
				//	snow[i].y += 20.0f;
				//}
				if (snow[i].y - cam_pos.y > 10.0f)
				{
					snow[i].y -= 20.0f;
				}

				if (snow[i].y - cam_pos.y < -10.0f)
				{
					snow[i].y += 20.0f;
				}



				if (snow[i].x - cam_pos.x > 20.0f)
				{
					snow[i].x -= 40.0f;
				}

				if (snow[i].x - cam_pos.x < -20.0f)
				{
					snow[i].x += 40.0f;
				}

				if (snow[i].z - cam_pos.z > 20.0f)
				{
					snow[i].z -= 40.0f;
				}

				if (snow[i].z - cam_pos.z < -20.0f)
				{
					snow[i].z += 40.0f;
				}




			}

		}
		//p_Level->

		//float noise_scale = 0.01f;

		//float noises = pn->noise(noise_position, 0.0f, 0.8)*100.0f;
		//int num_noise = (int)pn->noise(noise_position, 0.0f, 0.8)*100.0f;


		//noise_position += 0.1f;
		//if (noise_position > 100.0f)
		//{
		//	noise_position = 0.0f;
		//}



		//std::no

		int total_particles=0;

		current_index = 0;
		p_index.clear();
		part_index pi;
		for (i = 0; i<m_maxParticles; i++)
		{
			if (snow[i].bActive == 1)
			{
				float full_dist;
				float dist = m_Resources->m_Camera->CheckPoint(snow[i].x, snow[i].y, snow[i].z, 1.0f, &snow[i].full_dist);

				float t_height = snow[i].y - p_Level->GetTerrainHeight(snow[i].x, snow[i].z);

				total_particles++;

				if (dist > 0.0f && t_height>0.0f && snow[i].y > 1.0f && snow[i].full_dist>5.0f)
				{
					pi.part_no = i;
					pi.dist = dist;

					p_index.push_back(pi);

					//p_index[current_index].part_no = i;
					//p_index[current_index].dist = dist;
					current_index++;

				}
			}
		}

		if(total_particles+1 < MAX_FOG_PARTICLES)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, 400);

			for (i = 0; i < 100; i++)
			{
				float randx;
				float randz;
				float randy;

				float rx = (dist(gen) - 200)*0.1f;
				float rz = (dist(gen) - 200)*0.1f;
				float ry = (dist(gen) - 200)*0.05f;

				randz = cam_pos.z + rz;// +(m_Resources->m_Camera->LookingZ()*15.0f);
				randx = cam_pos.x + rx;// +(m_Resources->m_Camera->LookingX()*15.0f);
				randy = cam_pos.y + ry;// +(m_Resources->m_Camera->LookingX()*15.0f);
				if (true)//((rand() % (int)noise)>39)
				{
					CreateOne(randx, randy, randz);
				}
			}
		}

		if (current_index>0)
		{
			std::sort(p_index.begin(), p_index.end(), fog_compare{});

			//qsort(p_index, current_index, sizeof(part_index), SnowSortCB);
		}

	});

	//CreateVerticies();
}



void Fog::Render()
{
	//UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

concurrency::task<void> Fog::CreateVerticies()
{
	auto this_task = concurrency::create_task([this]
	{
		int index, i, j;

		XMFLOAT4 col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		float size = 4.5f;
		float snow_edge = 0.8f;

		XMVECTOR vtop_right = XMLoadFloat3(&XMFLOAT3(size, size, 0.0f));
		XMVECTOR vtop_left = XMLoadFloat3(&XMFLOAT3(-size, size, 0.0f));
		XMVECTOR vbottom_right = XMLoadFloat3(&XMFLOAT3(size, -size, 0.0f));
		XMVECTOR vbottom_left = XMLoadFloat3(&XMFLOAT3(-size, -size, 0.0f));

		XMFLOAT3 rtop_right;
		XMFLOAT3 rtop_left;
		XMFLOAT3 rbottom_right;
		XMFLOAT3 rbottom_left;

		XMFLOAT4X4 x_mat = *m_Resources->m_Camera->GetViewMatrix();

		XMMATRIX vrot_mat = XMLoadFloat3x3(&XMFLOAT3X3(x_mat._11, x_mat._12, x_mat._13, x_mat._21, x_mat._22, x_mat._23, x_mat._31, x_mat._32, x_mat._33));

		XMStoreFloat3(&rtop_right, XMVector3Transform(vtop_right, vrot_mat));
		XMStoreFloat3(&rtop_left, XMVector3Transform(vtop_left, vrot_mat));
		XMStoreFloat3(&rbottom_right, XMVector3Transform(vbottom_right, vrot_mat));
		XMStoreFloat3(&rbottom_left, XMVector3Transform(vbottom_left, vrot_mat));

		m_vertexCount = 0;

		// topleft
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rtop_left.x, rtop_left.y, rtop_left.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Top right.
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rtop_right.x, rtop_right.y, rtop_right.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom right.
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rbottom_right.x, rbottom_right.y, rbottom_right.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// topleft
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rtop_left.x, rtop_left.y, rtop_left.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom right.
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rbottom_right.x, rbottom_right.y, rbottom_right.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;
		// Bottom left.
		m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(rbottom_left.x, rbottom_left.y, rbottom_left.z);
		m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 1.0f);
		m_vertices[m_vertexCount].col = col;
		m_vertexCount++;

		m_indexCount = 6;

		m_instances.clear();

		for (i = 0; i < current_index; i++)
		{
			if (snow[p_index[i].part_no].z > p_Level->front_back_walls || snow[p_index[i].part_no].z < -p_Level->front_back_walls)
				continue;

			ParticleInstance pinsatnce;

			pinsatnce.position = XMFLOAT3(snow[p_index[i].part_no].x, snow[p_index[i].part_no].y, snow[p_index[i].part_no].z);
			GetLightAtPosition(pinsatnce.position, pinsatnce.color);

			pinsatnce.color.x = 0.2f + pinsatnce.color.x;
			pinsatnce.color.y = 0.2f + pinsatnce.color.y;
			pinsatnce.color.z = 0.2f + pinsatnce.color.z;

			pinsatnce.color.w = ((pinsatnce.color.x + pinsatnce.color.y + pinsatnce.color.z)*0.05f)*snow[p_index[i].part_no].alpha;

			m_instances.push_back(pinsatnce);
		}
	});

	return this_task;
}


