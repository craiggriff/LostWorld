#pragma once
#include <vector>


//#include "GameHelpers.h"
//#include "Constants.h"


#include "../DirectXTK/Inc/DDSTextureLoader.h"

#include "Physics.h"


#include "Level.h"


#include "Splash.h"

#include "PerlinNoise.h"
#include "Sparticle.h"

typedef struct
{
	int bActive;
	bool bInView;
	float dist;


	float x, y, z;
	float last_x, last_y, last_z;

	float life;
	float v1, v2; // texture offsey
	float vel_x, vel_y, vel_z;
	float nx, ny, nz;
	float angle, angle_mom;
	float floor_height;

	int flaketype;
	int stopped;
	float t_height;
} rain_t;



namespace Game
{

	class Rain : Sparticle
	{
	public:
		Rain(AllResources* p_Resources) : Sparticle(p_Resources) {};
		~Rain(void);

		std::vector<part_index> p_index;

		rain_t* rain;

		Splash* p_Splash;

		void Reset();

		int total_pills;
		int total_collected;

		DirectX::XMFLOAT4X4 matCamLast;
		DirectX::XMFLOAT4X4 matCamCurrent;


		bool bUpdate;

		int current_index;

		PerlinNoise* pn;

		DirectX::XMFLOAT3 cam_last_pos;

		int current_point;

		float rain_lightness;

		float noise_position;

		Level* p_Level;

		int current_particle_que;

		void Initialize();
		void UpdateVertexBuffers() { UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext()); };

		void SetRainLigtness(float _rain_lightness);

		float angle;
		float m_particleSize;

		void ActivateParticles(float posx, float posy, float posz);

		bool LoadLocations();

		void Initialize(Level* pp_Level, bool _bInstanced = false);


		void Render();

		concurrency::task<void> Rain::CreateVerticies();

		Concurrency::task<void> Update(float timeTotal, float timeDelta);


		void CreateOne(float x, float y, float z);



	};

}