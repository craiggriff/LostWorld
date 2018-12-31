#include "pch.h"
#include "rain.h"
#include "PerlinNoise.h"
#include "DefParticle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;

// std::sort with inlining
struct rain_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};


int RainSortCB(const VOID* arg1, const VOID* arg2)
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

void Rain::Reset()
{
	for (int i = 0; i<m_maxParticles; i++)
	{
		rain[i].bActive = 0;
	}

}


void Rain::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_RAIN_PARTICLES;

	rain = new rain_t[m_maxParticles];

	p_index.clear();

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;



	current_index = 0;

	current_point = 0;

	total_collected = 0;

	noise_position = 0.0f;

	rain_lightness = 1.0f;

	InitializeBuffers(_bInstanced);

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"Assets/Textures/drop.dds", nullptr, &m_Texture, MAXSIZE_T);

	m_Texture = m_Resources->m_Textures->LoadTexture("drop");

	for (int i = 0; i<m_maxParticles; i++)
	{
		rain[i].bActive = 0;
	}

}



void Rain::CreateOne(float x, float y, float z)
{
	int point_find = 0;
	point_find = current_point;
	do
	{
		if (point_find>m_maxParticles - 2)
			point_find = 0;

		if (rain[point_find].bActive == 0)
		{
			rain[point_find].bActive = 1;
			rain[point_find].vel_y = 2.0f + (rand() % 50) / 250.0f;
			rain[point_find].x = x;
			rain[point_find].y = y;
			rain[point_find].z = z;
			rain[point_find].v1 = 0.125f*(float)(rand() % 8);
			rain[point_find].v2 = rain[point_find].v1 + 0.125f;
			rain[point_find].life = 100.0f;
			rain[point_find].stopped = 0;
			if (false)//(abs(rain[current_point].z) < 4.0f)
			{
				rain[point_find].floor_height = 0.0f;
			}
			else
			{
				rain[point_find].floor_height = p_Level->GetTerrainHeight(rain[point_find].x, rain[point_find].z);
				if (rain[point_find].floor_height < 1.0f)
					rain[point_find].floor_height = 1.0f;
			}
			point_find++;
			break;
		}
		else
		{
			point_find++;
		}
			


	} while (point_find < m_maxParticles - 2 && point_find!= current_point);

	current_point = point_find;
	if (current_point>m_maxParticles - 2)
		current_point = 0;
}




Rain::~Rain(void)
{


}

Concurrency::task<void> Rain::Update(float timeTotal, float timeDelta)
{
	return Concurrency::create_task([this, timeDelta]
	{
		int i, j;
		std::random_device rd;
		srand(rd());

		//timeDelta *= 1.1f;

		unsigned int seed = 237;
		float pnscale = m_Resources->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);


		
		

		//angle=angle+0.01f;
		XMFLOAT4 col = DirectX::XMFLOAT4(0.8f, 0.8f, 1.0f, 1.0f);

		XMFLOAT3 cam_pos = m_Resources->m_Camera->GetEye();

		cam_pos.x += m_Resources->m_Camera->LookingX()*15.0f;
		cam_pos.z += m_Resources->m_Camera->LookingZ()*15.0f;

		for (i = 0; i < m_maxParticles; i++)
		{

			
			if (rain[i].bActive == 1)
			{

				//rain[i].last_x = rain[i].x +cam_pos.x - cam_last_pos.x;
				//rain[i].last_y = rain[i].y +cam_pos.y - cam_last_pos.y;
				//rain[i].last_z = rain[i].z +cam_pos.z - cam_last_pos.z;


				float diff_x = -m_Resources->m_LevelInfo.wind.x*timeDelta*0.02f;
				float diff_y = -rain[i].vel_y*timeDelta*8.0f;
				float diff_z = -m_Resources->m_LevelInfo.wind.z*timeDelta*0.02f;

				float speed = ((pn.noise((rain[i].x + diff_x)*pnscale, (rain[i].z + diff_z)*pnscale, (rain[i].z + diff_y)*pnscale)))*wave_height;
				diff_x = diff_x * speed;
				diff_z = diff_z * speed;

				rain[i].y += diff_y;

				if (false)//(rain[i].y <= rain[i].floor_height)
				{
					//	rain[i].vel_y = 0.0f;
					rain[i].bActive = 0;

					rain[i].y = p_Level->GetTerrainHeight(rain[i].x, rain[i].z);
					if (rain[i].y < 1.0f)
					{
						rain[i].y = 1.0f;
					}

					if (rain[i].y > 1.0f)
					{
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);
						p_Splash->CreateOne(rain[i].x, rain[i].y, rain[i].z, col, 2);

					}
					//
				}
				else
				{
					if (rain[i].life == 100.0f)
						rain[i].life = 99.0f;

					rain[i].x += diff_x;
					rain[i].z += diff_z;

					btVector3 vec_diff = btVector3(diff_x + cam_last_pos.x - cam_pos.x, diff_y + cam_last_pos.y - cam_pos.y, diff_z + cam_last_pos.z - cam_pos.z);
					vec_diff.normalize();

					rain[i].last_x = rain[i].x - vec_diff.getX()*0.5f;
					rain[i].last_y = rain[i].y - vec_diff.getY()*0.5f;
					rain[i].last_z = rain[i].z - vec_diff.getZ()*0.5f;

					if (rain[i].y - cam_pos.y > 10.0f)
					{
						rain[i].y -= 20.0f;
						rain[i].last_y -= 20.0f;
					}
					
					if (rain[i].y - cam_pos.y < -10.0f)
					{
						rain[i].y += 20.0f;
						rain[i].last_y += 20.0f;
					}



					if (rain[i].x - cam_pos.x > 20.0f)
					{
						rain[i].x -= 40.0f;
						rain[i].last_x -= 40.0f;
					}
					
					if (rain[i].x - cam_pos.x < -20.0f)
					{
						rain[i].x += 40.0f;
						rain[i].last_x += 40.0f;
					}

					if (rain[i].z - cam_pos.z > 20.0f)
					{
						rain[i].z -= 40.0f;
						rain[i].last_z -= 40.0f;
					}

					if (rain[i].z - cam_pos.z < -20.0f)
					{
						rain[i].z += 40.0f;
						rain[i].last_z += 40.0f;
					}
				}
			}
		}

		cam_last_pos = cam_pos;

		//matCamLast = *m_Resources->m_Camera->GetViewMatrix();

		/*
		btVector3 player_direct = *p_Camera->GetLookingDirection();
		//btVector3 player_position = p_Camera->Eye();
		btVector3 rand_point;// = btVector3(((float)(rand() % 50))) - 25.0f, 20.0f, ((float)(rand() % 50)) - 25.0f);
		XMFLOAT4X4 x_mat = *p_Camera->GetViewMatrix();
		btVector3 tran;
		btQuaternion quat_rot;
		btMatrix3x3 rot_mat = btMatrix3x3(x_mat._11, x_mat._12, x_mat._13, x_mat._21, x_mat._22, x_mat._23, x_mat._31, x_mat._32, x_mat._33);
		for (i = 0; i < 30; i++)
		{
		rand_point = btVector3(((float)(rand() % 50)) - 25.0f, 20.0f, ((float)(rand() % 50)) - 25.0f);
		rand_point = rand_point*rot_mat;
		CreateOne(x_mat._14 + rand_point.getX(), 20.0f, x_mat._34 + rand_point.getZ());
		}
		*/




		//float noise_scale = 0.01f;

		//float noises = pn->noise(noise_position, 0.0f, 0.8)*100.0f;
		//int num_noise = (int)pn->noise(noise_position, 0.0f, 0.8)*100.0f;


		//noise_position += 0.1f;
		//if (noise_position > 100.0f)
		//{
		//	noise_position = 0.0f;
		//}
		for (i = 0; i < 50; i++)
		{
			float randx;
			float randz;

			float rx = ((rand() % 1000) - 500)*0.02f;
			float rz = ((rand() % 1000) - 500)*0.02f;

			randz = cam_pos.z + rz;// (m_Resources->m_Camera->LookingZ()*rz);
			randx = cam_pos.x + rx; // (m_Resources->m_Camera->LookingX()*rx);

			if (true)//((rand() % (int)noise)>39)
			{
				CreateOne(randx, 20.0f, randz);
			}
		}

		current_index = 0;
		p_index.clear();
		part_index pi;
		for (i = 0; i<m_maxParticles; i++)
		{
			if (rain[i].bActive == 1)
			{
				float full_dist;
				float dist = m_Resources->m_Camera->CheckPoint(rain[i].x, rain[i].y, rain[i].z, 1.0f, &full_dist);

				float t_height = rain[i].y - p_Level->GetTerrainHeight(rain[i].x, rain[i].z);

				if (dist > 0.0f && t_height>0.0f)
				{
					pi.dist = dist;
					pi.part_no = i;
					p_index.push_back(pi);

					//p_index[current_index].part_no = i;
					//p_index[current_index].dist = dist;
					current_index++;
					
				}
			}
		}

		if (current_index>0)
		{
			std::sort(p_index.begin(), p_index.end(), rain_compare{});
			//qsort(p_index, current_index, sizeof(part_index), RainSortCB);
		}

	});

	//CreateVerticies();
}



void Rain::Render()
{
	//UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

void Rain::SetRainLigtness(float _rain_lightness)
{
	rain_lightness = _rain_lightness;

}

concurrency::task<void> Rain::CreateVerticies()
{

	auto this_task = concurrency::create_task([this]
	{
		XMVECTOR dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);
		XMVECTOR amb = XMLoadFloat4(&m_Resources->m_LevelInfo.ambient_col);

		int index, i,j;
		int particle_count = 0;

		index = 0;
		int total_active = 0;
		float px;
		float py;
		float pz;

		float lpx;
		float lpy;
		float lpz;

		DirectX::XMFLOAT4 col;
		float size = 0.1f;
		float sizeB = 0.0f;
		
		float xsize = m_Resources->m_Camera->view_tan.x*0.01f;
		float zsize = m_Resources->m_Camera->view_tan.z*0.01f;

		m_vertexCount = 0;
		m_indexCount = 0;
	
		//particle_count=1;
		if (true) //(total_active<(m_maxParticles*20)-1)
		{
			//bUpdate=false;
			for (i = 0; i<current_index; i++)
			{
				//=m_particleSize;

				//if ()
				//continue;

				px = rain[p_index[i].part_no].x;
				py = rain[p_index[i].part_no].y;
				pz = rain[p_index[i].part_no].z;
				if (rain[p_index[i].part_no].life == 100.0f)
				{
					lpx = rain[p_index[i].part_no].x;
					lpy = rain[p_index[i].part_no].y;
					lpz = rain[p_index[i].part_no].z;
				}
				else
				{
					lpx = rain[p_index[i].part_no].last_x;
					lpy = rain[p_index[i].part_no].last_y;
					lpz = rain[p_index[i].part_no].last_z;
				}

				//lpx = px + 1.0f;
				//lpy = py + 1.0f;
				//lpz = pz + 1.0f;



				GetLightAtPosition(XMFLOAT3(px,py,pz), col);
				col.w = (col.x + col.y + col.z)*0.5f;
				/*
				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
				output.color += ((diffuseColor)* 0.5);

				// Saturate the ambient and diffuse color.
				output.color = saturate(output.color);


				for (int i = 0; i < numLights; ++i)
				{
				l = (lights[i].pos - positionW.xyz) / lights[i].radius;

				if (length(l) < lights[i].radius)
				{
				float atten = saturate(1.0f - dot(l, l));

				output.color += ((lights[i].diffuse + lights[i].ambient) *atten);// +(atten * lights[i].ambient);
				}
				}*/
				//if (col.x > 1.0f)
				//	col.x = 1.0f;
				//if (col.y > 1.0f)
				//	col.y = 1.0f;
				//if (col.z > 1.0f)
				//	col.z = 1.0f;
				//col.w = 0.4f;

				//UINT pack_col = unsigned char(col.z * 255.0f) |
				//unsigned char(col.y * 255.0f) << 8 |
				//unsigned char(col.x * 255.0f) << 16 |
				//unsigned char(col.w * 255.0f) << 24;



				index = m_vertexCount;

				// Top left.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;
				// Bottom right.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 1.0f);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;
				// Bottom left.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 1.0f);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;

				// Top right.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 0.0f);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;

				m_indexCount += 6;
				/*
				m_indices[m_indexCount++] = index;
				m_indices[m_indexCount++] = index + 1;
				m_indices[m_indexCount++] = index + 2;
				m_indices[m_indexCount++] = index;
				m_indices[m_indexCount++] = index + 3;
				m_indices[m_indexCount++] = index + 1;
				*/

			}
		}
	});

	return this_task;
}
