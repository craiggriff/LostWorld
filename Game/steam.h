#pragma once
#include <vector>


//#include "GameHelpers.h"a
//#include "Constants.h"

#include "../DirectXTK/Inc/DDSTextureLoader.h"

#include "Physics.h"

#include "Camera.h"

#include "Level.h"

#include "Sparticle.h"



typedef struct
{
	int bActive;

	btVector3 pos;
	btVector3 mom;

	float life;
	float v1, v2; // texture offsey

	float angle, angle_mom;
	int flaketype;
	int stopped;
	float size;
	int start_delay;
	float nx, ny, nz;
	DirectX::XMFLOAT4 col;

} steam_t;



namespace Game
{

	class steam : Sparticle
	{
	public:
		steam(AllResources* p_Resources) : Sparticle(p_Resources) {};
		~steam(void);


		std::vector<part_index> p_index;

		steam_t* spark;

		int total_pills;
		int total_collected;

		bool bUpdate;

		int current_index;

		int current_point;

		Level* p_Level;

		float splash_lightness;
		void SetSplashLigtness(float _splash_lightness);

		int current_particle_que;


		float angle;
		float m_particleSize;


		void Initialize(Level* pp_Level, bool _bInstanced = false);

		void Render();
		void UpdateVertexBuffers() { UpdateVertecies(m_Resources->m_deviceResources->GetD3DDeviceContext()); };

		Concurrency::task<void> Update(float timeTotal, float timeDelta);

		void steam::CreateOne(float x, float y, float z, float mx, float my, float mz, DirectX::XMFLOAT4 col, int start_delay);

		concurrency::task<void> steam::CreateVerticies();

	};

}