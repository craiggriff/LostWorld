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
	float topx, topy, topz;
	float last_x, last_y, last_z;
	DirectX::XMFLOAT4 color;

	float life;
	float v1, v2; // texture offsey
	float vel_x, vel_y, vel_z;
	float nx, ny, nz;
	float angle, angle_mom;
	float floor_height;
	float time_offset;
	float scale;

	int flaketype;
	int stopped;
	float t_height;

	float full_dist; // full distance from camera
} fire_t;



namespace Game
{

	class Fire : Sparticle
	{
	public:
		Fire(AllResources* p_Resources) : Sparticle(p_Resources) { };
		~Fire(void);

		std::vector<part_index> p_index;

		fire_t* grass;

		Splash* p_Splash;


		void Reset();
		float SetFire(float x, float y, float z, float _scale, float timeTotal, float timeDelta, DirectX::XMFLOAT4 _col);

		int total_pills;
		int total_collected;

		bool bUpdate;

		int current_index;

		float wind_pos;

		float timeTotal;

		float x_mom, z_mom;

		float change, x_movement, z_movement;

		PerlinNoise* pn;

		DirectX::XMFLOAT3 cam_last_pos;

		int current_point;

		float rain_lightness;

		float noise_position;

		Level* p_Level;

		int current_particle_que;

		void Initialize(Level* pp_Level, bool _bInstanced = false);
		void UpdateVertexBuffers() { UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext()); };

		void SetRainLigtness(float _rain_lightness);

		float angle;
		float m_particleSize;



		void Render();

		concurrency::task<void> Fire::CreateVerticies();

		Concurrency::task<void> Update(DX::StepTimer const& timer);


		void CreateOne(float x, float y, float z, float _topx, float _topy, float _topz, float _scale, DirectX::XMFLOAT4 _col);



	};

}