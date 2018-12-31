#include "pch.h"
#include "grass.h"
#include "PerlinNoise.h"
#include "DefParticle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;
using namespace std;

// std::sort with inlining
struct grass_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

int GrassSortCB(const VOID* arg1, const VOID* arg2)
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

unsigned long Grass::localrand()
{
	rand_state = rand_state * 46781267 + 9788973;  // scramble the number
	return rand_state;
}

void Grass::Reset()
{
	for (int i = 0; i<m_maxParticles; i++)
	{
		grass[i].bActive = 0;
	}

}


void Grass::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_GRASS_PARTICLES;

	grass = new grass_t[m_maxParticles];

	p_index.clear();

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;



	current_index = 0;

	current_point = 0;



	InitializeBuffers(_bInstanced);

	wind_pos = 0.0f;

	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"grass-blades.cmo", nullptr, &m_Texture, MAXSIZE_T);
	m_Texture = m_Resources->m_Textures->LoadTexture("grasssoft2");
	for (int i = 0; i<m_maxParticles; i++)
	{
		grass[i].bActive = 0;
	}

}



void Grass::CreateOne(float x, float y, float z, float _topx, float _topy, float _topz, float _scale, float _angle, int _flaketype, float _full_dist)
{
	int point_find = 0;

	if (current_point < (m_maxParticles / 2) - 2)
	{
		grass[current_point].bActive = 1;
		grass[current_point].vel_y = 0.0f;
		grass[current_point].x = x;
		grass[current_point].y = y;
		grass[current_point].z = z;
		grass[current_point].v1 = 0.125f;// *(float)(rand() % 8);
		grass[current_point].v2 = grass[current_point].v1 + 0.125f;
		grass[current_point].life = 100.0f;
		grass[current_point].stopped = 0;
		grass[current_point].scale = _scale;
		grass[current_point].topx = _topx;
		grass[current_point].topy = _topy;
		grass[current_point].topz = _topz;
		grass[current_point].flaketype = _flaketype;
		grass[current_point].angle = _angle;
		grass[current_point].full_dist = _full_dist;

		current_point++;
	}

}




Grass::~Grass(void)
{


}

Concurrency::task<void> Grass::Update(float timeTotal, float timeDelta)
{
	x_mom = m_Resources->m_LevelInfo.wind.x;// (timeTotal*m_Resources->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Resources->m_LevelInfo.wind.z; // (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.4f;

	wind_pos += 0.1f;
	return Concurrency::create_task([this, timeDelta, timeTotal]
	{
		int i, j, k;

		unsigned int seed = 237;
		float pnscale = m_Resources->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);

		float noise_z = 0.5f;

		float change, x_movement, z_movement;

		x_movement = (timeTotal*x_mom)*1.7f;
		z_movement = (timeTotal*z_mom)*1.7f;

		change = timeTotal*0.5f;

		current_point = 0;
		for (k = 0; k < p_Level->current_index; k++)
		{
			if (p_Level->m_Terrain[p_Level->p_index[k].part_no]->cam_check_point < 70.0f && p_Level->m_Terrain[p_Level->p_index[k].part_no]->cam_check_point > 0.0f)
			{
				srand(100);

				for (i = 0; i < 8; i++)
				{
					for (j = 0; j < 8; j++)
					{
						if (p_Level->m_Terrain[p_Level->p_index[k].part_no]->tex_blend[5][i][j] > 0.0f)
						{
							int numb = int(p_Level->m_Terrain[p_Level->p_index[k].part_no]->tex_blend[5][i][j] * 12.0f);
							for (int z = 0; z < numb; z++)
							{

								float y_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->height_map[i][j];

								float x_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->pos_x - 8.0f + 1.0f + (float)i*2.0f;
								float z_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->pos_z - 8.0f + 1.0f + (float)j*2.0f;

								x_pos += (rand() % 20)*0.1f;
								z_pos += ((rand() % 20)*0.1f);
								angle = (rand() % 20)*0.1f;
								int flaketype = rand() % 8;
								float full_dist;

								if (m_Resources->m_Camera->CheckPoint(x_pos, y_pos, z_pos, 2.0f, &full_dist)>0.0f)
								{
									if (full_dist < 60.0f)
									{
										float dist_scale = 1.0f;

										if (full_dist > 40.0f)
											dist_scale = (60.0f - full_dist)*0.05f;

										float add_val = (pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale, change));
										float x_pos2 = x_pos - (add_val*p_Level->m_Terrain[p_Level->p_index[k].part_no]->tex_blend[5][i][j]);
										float z_pos2 = z_pos - (add_val*p_Level->m_Terrain[p_Level->p_index[k].part_no]->tex_blend[5][i][j]);
										y_pos = p_Level->GetTerrainHeight(x_pos, z_pos);
										if (y_pos > 1.0f)
										{
											CreateOne(x_pos, y_pos, z_pos, x_pos2, y_pos, z_pos2, p_Level->m_Terrain[p_Level->p_index[k].part_no]->tex_blend[5][i][j] * dist_scale, angle, flaketype, full_dist);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		current_index = 0;
		p_index.clear();
		part_index pi;
		for (i = 0; i<current_point; i++)
		{
			pi.dist = grass[i].full_dist;
			pi.part_no = i;
			p_index.push_back(pi);
			current_index++;
		}

		if (current_index>0)
		{
			std::sort(p_index.begin(), p_index.end(), grass_compare{});
		}


	}).then([this]
	{

		return CreateVerticies();
	});
}



void Grass::Render()
{
	//UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}


concurrency::task<void> Grass::CreateVerticies()
{

	auto this_task = concurrency::create_task([this]
	{
		XMVECTOR dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);
		XMVECTOR amb = XMLoadFloat4(&m_Resources->m_LevelInfo.ambient_col);

		int index, i, j;

		index = 0;
		int total_active = 0;
		float px;
		float py;
		float pz;

		float lpx;
		float lpy;
		float lpz;

		DirectX::XMFLOAT4 col;
		float size = 0.03f;
		float sizeB = 0.0f;

		float scale = 0.5f;
		float scaley = 0.8f;

		float xsize;
		float zsize;

		m_vertexCount = 0;
		m_indexCount = 0;

	
		if (true)
		{
			for (i = 0; i < current_index; i++)
			{
				xsize = (1.0f*scale);
				zsize = ((-1.0f+grass[p_index[i].part_no].angle)*scale);

				px = grass[p_index[i].part_no].x;
				py = grass[p_index[i].part_no].y;
				pz = grass[p_index[i].part_no].z;

				lpx = grass[p_index[i].part_no].topx;
				lpy = py + scaley * grass[p_index[i].part_no].scale;
				lpz = grass[p_index[i].part_no].topz;

				col = m_Resources->m_Lights->m_lightBufferData.ambientColor;
				col.x += m_Resources->m_Lights->m_lightBufferData.diffuseColor.x *0.7f;
				col.y += m_Resources->m_Lights->m_lightBufferData.diffuseColor.y *0.7f;
				col.z += m_Resources->m_Lights->m_lightBufferData.diffuseColor.z *0.7f;

				col.w = 1.0f;

				if (col.w > 0.0f)
				{

					col.w = 1.0f;



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


					xsize = ((1.0f - grass[p_index[i].part_no].angle)*scale);
					zsize = (1.0f*scale);

					px = grass[p_index[i].part_no].x;
					py = grass[p_index[i].part_no].y;
					pz = grass[p_index[i].part_no].z;

					lpx = grass[p_index[i].part_no].topx;
					lpy = py + scaley * grass[p_index[i].part_no].scale;
					lpz = grass[p_index[i].part_no].topz;



					
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
					

				}
			}
		}
	});

	return this_task;
}
