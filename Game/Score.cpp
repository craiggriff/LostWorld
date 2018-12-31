#include "pch.h"
#include "Score.h"
#include "DefParticle.h"


using namespace Game;
using namespace DX;
using namespace DirectX;


// std::sort with inlining
struct score_compare {
	// the compiler will automatically inline this
	bool operator()(const part_index &a, const part_index &b) {
		return a.dist < b.dist;
	}
};


void Score::CreateOne(float x, float y, float z, float mx, float my, float mz, DirectX::XMFLOAT4 col, int start_delay)
{
	int point_find = 0;

	do
	{
		point_find++;

		if (true)//(spark[current_point].bActive == 0)
		{
			spark[current_point].bActive = 1;
			//spark[current_point].mom.setX(1.5f - (rand() % 150) / 50.0f);
			spark[current_point].mom.setX(mx);
			spark[current_point].mom.setY(my + (rand() % 150) / 30.0f);
			//spark[current_point].mom.setZ(1.5f - (rand() % 150) / 50.0f);
			spark[current_point].mom.setZ(mz);
			spark[current_point].pos.setX(x);
			spark[current_point].pos.setY(y);
			spark[current_point].pos.setZ(z);
			spark[current_point].v1 = 0.125f*(float)(rand() % 8);
			spark[current_point].v2 = spark[current_point].v1 + 0.125f;
			spark[current_point].life = 0.1 + ((rand() % 30)*0.1f);
			spark[current_point].stopped = 0;
			spark[current_point].angle = 0.0f;
			spark[current_point].angle_mom = 0.1f - (float)((rand() % 20) / 100.0f);
			spark[current_point].size = 0.3f + (spark[current_point].mom.getY()*0.3f);

			spark[current_point].nx = 0.0f;
			spark[current_point].ny = -1.0f;
			spark[current_point].nz = 0.0f;


			spark[current_point].col = col;

			spark[current_point].start_delay = start_delay;

			break;
		}
		else
		{
			current_point++;
			if (current_point>m_maxParticles - 2)
				current_point = 0;
		}

	} while (point_find < m_maxParticles - 2);
	//Splash[current_point].v1 = 0.125f*(float)num;
	//Splash[current_point].v2 = Splash[current_point].v1 + 0.125f;

	current_point++;
	if (current_point>m_maxParticles - 2)
		current_point = 0;
}

void Score::Initialize(Level* pp_Level, bool _bInstanced)
{
	p_Level = pp_Level;

	spark = new score_t[1000];

	//p_index = new part_index[1000];
	p_index.clear();

	bUpdate = true;

	current_particle_que = 0;

	m_particleSize = 1.0f;

	m_maxParticles = 1000;

	current_index = 0;

	current_point = 0;

	total_collected = 0;

	InitializeBuffers(_bInstanced);

	//CreateDDSTextureFromFile((ID3D11Device *)m_deviceResources->GetD3DDevice(), L"smoke3.dds", nullptr, &m_Texture, MAXSIZE_T);

	m_Texture = m_Resources->m_Textures->LoadTexture("misc_smoke");

	for (int i = 0; i<m_maxParticles; i++)
	{
		spark[i].bActive = 0;
	}

}





Score::~Score(void)
{


}

Concurrency::task<void> Score::Update(float timeTotal, float timeDelta)
{
	return Concurrency::create_task([this, timeDelta]
	{

		int i;
		//timeDelta *= 1.1f;

		//angle=angle+0.01f;
		float mod_time = timeDelta * 0.1f;
		for (i = 0; i < m_maxParticles; i++)
		{
			if (spark[i].bActive == 1)
			{
				//spark[i].angle += (float(rand()%100)/100.0f)*M_PI*2.0f;
				spark[i].angle += spark[i].angle_mom * mod_time * 20.0f;
				if (spark[i].angle > M_PI*2.0f)
				{
					spark[i].angle -= M_PI*2.0f;
				}
				else
				{
					if (spark[i].angle < 0.0f)
					{
						spark[i].angle += M_PI*2.0f;
					}
				}

				if (true) //(spark[i].stopped == 1 && spark[i].bActive == 1)
				{
					spark[i].life -= 10.0f*mod_time;

					spark[i].size += spark[i].mom.getY() * 5.1f*mod_time;

					if (spark[i].life < 0)
					{
						spark[i].bActive = 0;
					}
				}

				if (spark[i].pos.getY() <= p_Level->GetTerrainHeight(spark[i].pos.getX(), spark[i].pos.getZ()))
				{
					//spark[i].

					if (spark[i].mom.getY() < 1.5f && spark[i].mom.getY() > -1.5f)
					{
						spark[i].stopped = 1;
						spark[i].mom.setX(0.0f);
						spark[i].mom.setY(0.0f);
						spark[i].mom.setZ(0.0f);
					}
					spark[i].pos.setY(p_Level->GetTerrainHeight(spark[i].pos.getX(), spark[i].pos.getZ()));
					spark[i].mom.setY(-spark[i].mom.getY());
					spark[i].mom.setY(spark[i].mom.getY() * 0.8f);
				}
				else
				{

				}
				if (true)
				{
					/*
					float diff_x = -m_Resources->m_LevelInfo.wind.x*timeDelta;
					float diff_y = -snow[i].vel_y*timeDelta*5.0f;
					float diff_z = -m_Resources->m_LevelInfo.wind.z*timeDelta;
					*/
					spark[i].pos.setX(spark[i].pos.getX() - m_Resources->m_LevelInfo.wind.x*mod_time);
					spark[i].pos.setZ(spark[i].pos.getZ() - m_Resources->m_LevelInfo.wind.z*mod_time);

					spark[i].pos.setY(spark[i].pos.getY() + spark[i].mom.getY()*mod_time);


					//spark[i].mom.setY(spark[i].mom.getY() - 25.0f*timeDelta);

					if (spark[i].pos.getY() < -10.0f)
					{
						spark[i].bActive = 0;
					}
				}
			}
		}


		current_index = 0;
		part_index pi;
		for (i = 0; i < m_maxParticles; i++)
		{
			float full_dist;
			if (spark[i].bActive == 1 && m_Resources->m_Camera->CheckPoint(spark[i].pos.getX(), spark[i].pos.getY(), spark[i].pos.getZ(), 1.0f, &full_dist) > -1.0f)
			{
				pi.part_no = i;
				pi.dist = full_dist;
				p_index.push_back(pi);

				//p_index[current_index].part_no = i;
				//p_index[current_index].dist = full_dist;

				current_index++;
			}
		}

		if (current_index > 0)
		{
			std::sort(p_index.begin(), p_index.end(), score_compare{});
			//qsort(p_index, current_index, sizeof(part_index), SteamSortCB);
		}

	});

	//CreateVerticies();

}



void Score::Render()
{
	//UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext());

	Sparticle::Render();
}

void Score::SetSplashLigtness(float _splash_lightness)
{
	splash_lightness = _splash_lightness;

}


concurrency::task<void> Score::CreateVerticies()
{



	auto this_task = concurrency::create_task([this]
	{
		XMVECTOR dif = XMLoadFloat4(&m_Resources->m_LevelInfo.diffuse_col);
		XMVECTOR amb = XMLoadFloat4(&m_Resources->m_LevelInfo.ambient_col);

		int index, i, j;

		int particle_count = 0;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result;
		VertexPositionTexCol* verticesPtr;



		//particle_count=1;
		memset(m_vertices, 0, (sizeof(VertexPositionTexCol) * (m_vertexCount)));

		m_vertexCount = 0;
		m_indexCount = 0;

		// Now build the vertex array from the particle list array.  Each particle is a quad made out of two triangles.
		index = 0;
		int total_active = 0;
		float px;
		float py;
		float pz;
		DirectX::XMFLOAT4 col;
		float size = 0.4f;
		float sizeB = 0.2f;

		float xsize = m_Resources->m_Camera->LookingTanX() * size;
		float zsize = m_Resources->m_Camera->LookingTanZ() * size;

		DirectX::XMFLOAT3 norm = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);

		//particle_count=1;
		if (true) //(total_active<(m_maxParticles*20)-1)
		{
			//bUpdate=false;
			for (i = 0; i < current_index; i++)
			{
				//if (spark[p_index[i].part_no].size>0.1f)
				//	spark[p_index[i].part_no].size -= 0.005f;

				size = spark[p_index[i].part_no].size*0.3f;
				sizeB = size*0.5f;

				xsize = m_Resources->m_Camera->LookingTanX() * size;
				zsize = m_Resources->m_Camera->LookingTanZ() * size;


				//=m_particleSize;
				float ang_cos = -0.5f * cos(spark[p_index[i].part_no].angle);
				float ang_sin = -0.5f * sin(spark[p_index[i].part_no].angle);

				float ang_cos2 = 0.5f * cos(spark[p_index[i].part_no].angle);
				float ang_sin2 = 0.5f * sin(spark[p_index[i].part_no].angle);

				px = spark[p_index[i].part_no].pos.getX();
				py = spark[p_index[i].part_no].pos.getY();
				pz = spark[p_index[i].part_no].pos.getZ();

				col = m_Resources->m_Lights->m_lightBufferData.ambientColor;
				col.x += m_Resources->m_Lights->m_lightBufferData.diffuseColor.x *0.5f;
				col.y += m_Resources->m_Lights->m_lightBufferData.diffuseColor.y *0.5f;
				col.z += m_Resources->m_Lights->m_lightBufferData.diffuseColor.z *0.5f;

				//col.x *= spark[p_index[i].part_no].col.x;
				//col.y *= spark[p_index[i].part_no].col.y;
				//col.z *= spark[p_index[i].part_no].col.z;

				col.w = 1.0f;

				if (spark[p_index[i].part_no].life<3.0f)
				{


					//xsize = spark[p_index[i].part_no].life;// *= ((10.0f + spark[p_index[i].part_no].life)*0.1f);
					//zsize = spark[p_index[i].part_no].life;// *= ((10.0f + spark[p_index[i].part_no].life)*0.1f);
					//sizeB = spark[p_index[i].part_no].life;// *= ((10.0f + spark[p_index[i].part_no].life)*0.1f);
					//col = DirectX::XMFLOAT4(spark[p_index[i].part_no].col.x, spark[p_index[i].part_no].col.y, spark[p_index[i].part_no].col.z, (spark[p_index[i].part_no].life*0.05f));
					//col = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, spark[p_index[i].part_no].life*0.1f);
					//spark[p_index[i].part_no].col.w = spark[p_index[i].part_no].life*0.1f;
					col.w = (spark[p_index[i].part_no].life*0.05f);
				}

				if (col.w > 0.0f)
				{

					if (m_Resources->bCPULightParticles == true)
						for (j = 0; j < m_Resources->m_Lights->m_pointLights.size(); j++)
						{
							if (m_Resources->m_Lights->m_pointLights[j].spot > 0.0f)
							{

								XMVECTOR l = (XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[j].pos) - XMLoadFloat3(&XMFLOAT3(px, py, pz)));// / m_Resources->m_Lights->m_pointLights[j].radius;
								float len;
								XMStoreFloat(&len, XMVector3Length(l));
								if (len < m_Resources->m_Lights->m_pointLights[j].radius)
								{
									float atten = (1.0f - (len / m_Resources->m_Lights->m_pointLights[j].radius)) * (1.0 / (1.0 + 0.1*len + 0.01*len*len)); // 1.0f - (len / m_Resources->m_Lights->m_pointLights[j].radius);
									float attenB;

									l = XMVector3Normalize(l);

									XMVECTOR ldif = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].diffuse);
									XMVECTOR lamb = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].ambient);
									XMVECTOR ldir = XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[j].dir);

									XMVECTOR spot = XMLoadFloat(&m_Resources->m_Lights->m_pointLights[j].spot);

									//float spotlight = (pow(max(dot(-l, lights[i].dir), 0.0f), lights[i].spot)*lightIntensity);
									//output.color += spotlight*atten*((lights[i].diffuse + lights[i].ambient)*2.0f);

									float nil = 0.0f;
									XMVECTOR vattenB = XMVectorPow(XMVectorMax(XMVector3Dot(-l, ldir), XMLoadFloat(&nil)), spot); // intensity1
									XMStoreFloat(&attenB, vattenB);

									atten = (atten*(attenB*1.0f));
									if (atten > 0.0f)
									{
										XMVECTOR out = ((ldif + lamb)*2.0f)*attenB*atten;// *(vattenB*10.0f));
										XMFLOAT4 light_col;
										XMStoreFloat4(&light_col, out);
										col.x += light_col.x;
										col.y += light_col.y;
										col.z += light_col.z;
									}
								}
							}
							else
							{
								XMVECTOR l = (XMLoadFloat3(&m_Resources->m_Lights->m_pointLights[j].pos) - XMLoadFloat3(&XMFLOAT3(px, py, pz)));// / m_Resources->m_Lights->m_pointLights[j].radius;
								float len;
								XMStoreFloat(&len, XMVector3Length(l));
								if (len < m_Resources->m_Lights->m_pointLights[j].radius)
								{
									float dot;
									XMStoreFloat(&dot, XMVector3Dot(l, l));
									float atten = 1.0 / (1.0 + 0.1*len + 0.01*len*len);



									XMVECTOR ldif = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].diffuse);
									XMVECTOR lamb = XMLoadFloat4(&m_Resources->m_Lights->m_pointLights[j].ambient);



									XMVECTOR out = ((ldif + lamb)*1.0f)*atten;
									XMFLOAT4 light_col;
									XMStoreFloat4(&light_col, out);
									col.x += light_col.x*0.2f;
									col.y += light_col.y*0.2f;
									col.z += light_col.z*0.2f;

									//col.w += light_col.w;
								}
							}
						}
				}

				//if (col.x > 1.0f)
				//	col.x = 1.0f;
				//if (col.y > 1.0f)
				//	col.y = 1.0f;
				//if (col.z > 1.0f)
				//	col.z = 1.0f;

				//col.w = 1.0f;

				//UINT pack_col = unsigned char(col.z * 255.0f) |
				//	unsigned char(col.y * 255.0f) << 8 |
				//	unsigned char(col.x * 255.0f) << 16 |
				//	unsigned char(col.w * 255.0f) << 24;


				total_active++;
				index = m_vertexCount;

				// Top left.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px - (xsize), py + sizeB, pz - (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.5f + (ang_cos - ang_sin), 0.5f + (ang_sin + ang_cos)/*spark[p_index[i].part_no].v1*/);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;
				// Bottom right.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px + (xsize), py - sizeB, pz + (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.5f + (ang_cos2 - ang_sin2), 0.5f + (ang_sin2 + ang_cos2)/*spark[p_index[i].part_no].v2*/);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;
				// Bottom left.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px - (xsize), py - sizeB, pz - (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.5f + (ang_cos - ang_sin2), 0.5f + (ang_sin + ang_cos2)/*spark[p_index[i].part_no].v2*/);
				m_vertices[m_vertexCount].col = col;
				m_vertexCount++;
				// Top right.
				m_vertices[m_vertexCount].pos = DirectX::XMFLOAT3(px + (xsize), py + sizeB, pz + (zsize));
				m_vertices[m_vertexCount].tex = DirectX::XMFLOAT2(0.5f + (ang_cos2 - ang_sin), 0.5f + (ang_sin2 + ang_cos)/*spark[p_index[i].part_no].v1*/);
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