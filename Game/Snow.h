#pragma once
#include <vector>



#include "../DirectXTK/Inc/DDSTextureLoader.h"

#include "Physics.h"

#include "Camera.h"

#include "Level.h"

#include "sparticle.h"

#include "PerlinNoise.h"


typedef struct
{
	int bActive;
	float x, y, z;

	float life;
	float v1, v2; // texture offsey
	float vel_x, vel_y, vel_z;
	float angle, angle_mom;
	int flaketype;
	int stopped;
	float t_height;
	float full_dist;
	float lightness;

} snow_t;



namespace Game
{

	class Snow : Sparticle
	{
	public:
		Snow(AllResources* p_Resources) : Sparticle(p_Resources) {};
		~Snow(void);

		std::vector<part_index> p_index;

		snow_t* snow;

		unsigned rand_state;// = 0;  // <- state of the RNG (input and output)


		int total_pills;
		int total_collected;

		bool bUpdate;

		int current_index;


		int current_point;

		Level* p_Level;

		PerlinNoise* pn;



		float noise_position;

		int current_particle_que;

		void Initialize(Level* pp_Level, bool _bInstanced = false);

		float angle;
		float m_particleSize;


		void Reset();
		unsigned localrand();

		void UpdateVertexBuffers() { UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext()); };

		void Render();

		Concurrency::task<void> Update(float timeTotal, float timeDelta);

		void CreateOne(float x, float y, float z);




		concurrency::task<void> Snow::CreateVerticies();

	};

}