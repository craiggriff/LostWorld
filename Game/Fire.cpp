#include "pch.h"
#include "Fire.h"
#include "PerlinNoise.h"
#include "DefParticle.h"

using namespace Game;
using namespace DX;
using namespace DirectX;
using namespace std;

// std::sort with inlining
struct fire_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};

int FireSortCB(const VOID* arg1, const VOID* arg2)
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


void Fire::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	m_maxParticles = MAX_FIRE_PARTICLES;

	grass = new fire_t[m_maxParticles];

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

	wind_pos = 0.0f;



	pn = new PerlinNoise(237);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"grass-blades.cmo", nullptr, &m_Texture, MAXSIZE_T);
	m_Texture = m_Resources->m_Textures->LoadTexture("fire01");
	m_Texture2 = m_Resources->m_Textures->LoadTexture("noise01");
	m_Texture3 = m_Resources->m_Textures->LoadTexture("alpha1");

	num_of_textures = 3;

	for (int i = 0; i<m_maxParticles; i++)
	{
		grass[i].bActive = 0;
	}

}



void Fire::CreateOne(float x, float y, float z, float _topx, float _topy, float _topz, float _scale,DirectX::XMFLOAT4 _col)
{
	int point_find = 0;

	

	do
	{
		point_find++;

		if (grass[current_point].bActive == 0)
		{
			grass[current_point].color= _col;
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
			grass[current_point].time_offset = (float)(rand() % 20)*0.9f;
			break;
		}
		else
		{
			current_point++;
			if (current_point>m_maxParticles - 2)
				current_point = 0;
		}

	} while (point_find < m_maxParticles - 2);
	//snow[current_point].v1 = 0.125f*(float)num;
	//snow[current_point].v2 = snow[current_point].v1 + 0.125f;

	current_point++;
	if (current_point>m_maxParticles - 2)
		current_point = 0;
}




Fire::~Fire(void)
{


}

void Fire::Reset()
{
	for (int k = 0; k < m_maxParticles; k++)
	{
		grass[k].bActive = 0;
	}

	current_point = 0;
}

float Fire::SetFire(float x, float y, float z, float _scale, float timeTotal, float timeDelta, DirectX::XMFLOAT4 _col)
{
	x_mom = m_Resources->m_LevelInfo.wind.x;// (timeTotal*m_Resources->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Resources->m_LevelInfo.wind.z; // (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.4f;

	unsigned int seed = 237;
	float pnscale = m_Resources->m_LevelInfo.wind.w*0.4f;
	float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
	PerlinNoise pn(seed);

	if (m_Resources->m_Camera->CheckPlanes(x, y, z, 2.0f) == true)
	{
		float add_val = (pn.noise(((float)(x)+x_movement)*pnscale, ((float)(z)+z_movement)*pnscale, change));
		float x_pos2 = x - (add_val*_scale);
		float z_pos2 = z - (add_val*_scale);
		//y = p_Level->GetTerrainHeight(x, z);
		if (y > 1.0f)
		{
			CreateOne(x, y, z, x_pos2, y+3.0f*_scale, z_pos2, _scale, _col);
		}
		return add_val;
	}
	return 0.0f;
}

Concurrency::task<void> Fire::Update(DX::StepTimer const& timer)
{
	x_mom = m_Resources->m_LevelInfo.wind.x;// (timeTotal*m_Resources->m_LevelInfo.wind.x)*0.4f;
	z_mom = m_Resources->m_LevelInfo.wind.z; // (timeTotal*m_Resources->m_LevelInfo.wind.z)*0.4f;

	wind_pos += 0.1f;
	return Concurrency::create_task([this, timer]
	{
		int i, j, k;
		//timeDelta *= 1.1f;

		DistortionBufferType db;
		db.distortion1 = XMFLOAT2(0.1f, 0.2f);
		db.distortion2 = XMFLOAT2(0.1f, 0.3f);
		db.distortion3 = XMFLOAT2(0.1f, 0.1f);
		db.distortionScale = 0.8f;
		db.distortionBias = 0.5f;
		m_Resources->UpdateDistortionBuffer(&db);


		NoiseBufferType nb;
		nb.frameTime = timer.GetTotalSeconds();
		nb.scrollSpeeds = XMFLOAT3(1.6f, 1.1f, 1.3f);
		nb.scales = XMFLOAT3(0.3f, 0.6f, 0.9f);
		nb.padding = 0.0f;
		m_Resources->UpdateNoiseBuffer(&nb);


		unsigned int seed = 237;
		float pnscale = m_Resources->m_LevelInfo.wind.w*0.4f;
		float wave_height = m_Resources->m_LevelInfo.wind.y*2.0f;
		PerlinNoise pn(seed);

		float noise_z = 0.5f;

		

		x_movement = (timeTotal*x_mom)*1.7f;
		z_movement = (timeTotal*z_mom)*1.7f;

		change = timeTotal*0.5f;

		/////////////////////////
		//
		// Create the grass
		//
		/////////////////////////
		/*
		for (k = 0; k < m_maxParticles; k++)
		{
			grass[k].bActive = 0;
		}

		current_point = 0;
		for (k = 0; k < p_Level->current_index; k++)
		{
			if (p_Level->m_Terrain[p_Level->p_index[k].part_no]->cam_check_point < 40.0f && p_Level->m_Terrain[p_Level->p_index[k].part_no]->cam_check_point > -1.0f)
			{
				srand(100);
				for (i = 0; i < 8; i++)
				{
					for (j = 0; j < 8; j++)
					{
						if (p_Level->m_Terrain[p_Level->p_index[k].part_no]->cols[i][j].w > 0.0f)
						{
							int numb = int(p_Level->m_Terrain[p_Level->p_index[k].part_no]->cols[i][j].w * 30.0f);
							for (int z = 0; z < numb; z++)
							{

								float y_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->height_map[i][j];

								float x_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->pos_x - 8.0f + 1.0f + (float)i*2.0f;
								float z_pos = p_Level->m_Terrain[p_Level->p_index[k].part_no]->pos_z - 8.0f + 1.0f + (float)j*2.0f;

								x_pos += (rand() % 20)*0.1f;
								z_pos += ((rand() % 20)*0.1f);// +(pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale, change))*wave_height;
															  //x_pos += (pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale, change))*wave_height;

								if (m_Resources->m_Camera->CheckPlanes(x_pos, y_pos, z_pos, 2.0f) == true)
								{
									float add_val = (pn.noise(((float)(x_pos)+x_movement)*pnscale, ((float)(z_pos)+z_movement)*pnscale, change));
									float x_pos2 = x_pos - (add_val*p_Level->m_Terrain[p_Level->p_index[k].part_no]->cols[i][j].w);
									float z_pos2 = z_pos - (add_val*p_Level->m_Terrain[p_Level->p_index[k].part_no]->cols[i][j].w);
									y_pos = p_Level->GetTerrainHeight(x_pos, z_pos);
									if (y_pos > 1.0f)
									{
										CreateOne(x_pos, y_pos, z_pos, x_pos2, y_pos, z_pos2, p_Level->m_Terrain[p_Level->p_index[k].part_no]->cols[i][j].w);
									}
								}
							}
						}

					}
				}
			}
		}
		*/
		current_index = 0;
		p_index.clear();
		part_index pi;
		for (i = 0; i<m_maxParticles; i++)
		{
			if (grass[i].bActive == 1)
			{
				float full_dist;
				float dist = m_Resources->m_Camera->CheckPoint(grass[i].x, grass[i].y, grass[i].z, 1.0f, &grass[i].full_dist);

				if (dist > 0.0f && grass[i].full_dist<130.0f)
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
			std::sort(p_index.begin(), p_index.end(), fire_compare{});
			//qsort(p_index, current_index, sizeof(part_index), FireSortCB);
		}


	}).then([this]
	{

		return CreateVerticies();
	});
}



void Fire::Render()
{
	//UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

void Fire::SetRainLigtness(float _rain_lightness)
{
	rain_lightness = _rain_lightness;

}

concurrency::task<void> Fire::CreateVerticies()
{

	auto this_task = concurrency::create_task([this]
	{
		XMVECTOR dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);
		XMVECTOR amb = XMLoadFloat4(&m_Resources->m_LevelInfo.ambient_col);

		int index, i, j;




		//particle_count=1;
		//memset(m_vertices, 0, (sizeof(VertexPositionTex) * (m_vertexCount)));

		// Now build the vertex array from the particle list array.  Each particle is a quad made out of two triangles.
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

		float xsize;// = m_Resources->m_PanCamera->vec_looking_tan.getX() *grass[p_index[i].part_no].scale;
		float zsize;// = m_Resources->m_PanCamera->vec_looking_tan.getZ()*grass[p_index[i].part_no].scale;
						   //xsize = -0.05f;
						   //zsize = -0.00005f;

		m_vertexCount = 0;
		m_indexCount = 0;

		//particle_count=1;
		if (true) //(total_active<(m_maxParticles*20)-1)
		{
			//bUpdate=false;
			for (i = 0; i < current_index; i++)
			{
				//=m_particleSize;

				//if ()
				//continue;
				
				xsize = m_Resources->m_Camera->LookingTanX()*grass[p_index[i].part_no].scale;
				zsize = m_Resources->m_Camera->LookingTanZ()*grass[p_index[i].part_no].scale;

				px = grass[p_index[i].part_no].x;
				py = grass[p_index[i].part_no].y;
				pz = grass[p_index[i].part_no].z;

				lpx = grass[p_index[i].part_no].topx;
				lpy = grass[p_index[i].part_no].topy;// *grass[p_index[i].part_no].scale;
				lpz = grass[p_index[i].part_no].topz;



				//col = DirectX::XMFLOAT4(rain_lightness, rain_lightness, rain_lightness, 0.2f);

				col.x = 1.0f;
				col.y = 1.0f;
				col.z = 1.0f;

				if (grass[p_index[i].part_no].full_dist < 30.0f)
				{

					if (grass[p_index[i].part_no].full_dist > 20.0f)
					{

						col.w = 1.0f - ((grass[p_index[i].part_no].full_dist - 20.0f)*0.1f);
					}
					else
					{
						col.w = 1.0f;
					}
				}
				else
				{
					col.w = 0.0f;
				}



					//if (col.x > 1.0f)
					//	col.x = 1.0f;
					//if (col.y > 1.0f)
					//	col.y = 1.0f;
					//if (col.z > 1.0f)
					//	col.z = 1.0f;


					//UINT pack_col = unsigned char(col.z * 255.0f) |
					//	unsigned char(col.y * 255.0f) << 8 |
					//	unsigned char(col.x * 255.0f) << 16 |
					//	unsigned char(col.w * 255.0f) << 24;

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
					}
					*/


					index = m_vertexCount;

					// Top left.
					m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(lpx - (xsize), lpy + sizeB, lpz - (zsize));
					m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
					m_vertices[m_vertexCount].col = grass[p_index[i].part_no].color;
					m_vertexCount++;
					// Bottom right.
					m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
					m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 1.0f);
					m_vertices[m_vertexCount].col = grass[p_index[i].part_no].color;
					m_vertexCount++;
					// Bottom left.
					m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
					m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.0f, 1.0f);
					m_vertices[m_vertexCount].col = grass[p_index[i].part_no].color;
					m_vertexCount++;

					// Top right.
					m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(lpx + (xsize), lpy + sizeB, lpz + (zsize));
					m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(1.0f, 0.0f);
					m_vertices[m_vertexCount].col = grass[p_index[i].part_no].color;
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
