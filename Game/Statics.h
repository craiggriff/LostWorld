#pragma once


using namespace std;


#include "mesh.h"
#include "animation.h"

#include "flag.h"
#include "fire.h"
#include "steam.h"


namespace Game
{

	typedef struct
	{
		std::vector<float> m_time; // for animations

		std::vector<Mesh*> m_meshModels;
		int type;
		float light_radius;
		int physics_shape;
		ObjInfo info;
		bool bHasAnim;

	} static_model_t;

	typedef struct
	{
	public:
		int bActive;
		float scale;
		float x, y, z;
		float rot_x, rot_y, rot_z;
		float height_from_terrain; // used for terrain changes
		DirectX::XMFLOAT4X4 model_matrix;
		int model_index;
		float flag_pos;
		float angle;
		bool bHasAnim;
		int type;
		float dist;
		DirectX::XMFLOAT4 colour;

		DirectX::XMFLOAT4 spare[50];

		void MakeMatrix()
		{
			DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rot_x, rot_y, rot_z);
			DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(x, y, z);

			DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale, scale, scale);
			DirectX::XMMATRIX scaleRotationMatrix = DirectX::XMMatrixMultiply(scaleMatrix, rotationMatrix);
			DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

			XMStoreFloat4x4(&model_matrix, modelMatrix);
		}

	} static_t;

	typedef struct
	{
	public:
		int bActive;
		float scale;
		float x, y, z;
		float rot_x, rot_y, rot_z;
		float height_from_terrain; // used for terrain changes
		DirectX::XMFLOAT4X4 model_matrix;
		int model_index;
		float flag_pos;
		float angle;
		bool bHasAnim;
		int type;
		float dist;


		void MakeMatrix()
		{
			DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rot_x, rot_y, rot_z);
			DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(x, y, z);

			DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale, scale, scale);
			DirectX::XMMATRIX scaleRotationMatrix = DirectX::XMMatrixMultiply(scaleMatrix, rotationMatrix);
			DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

			XMStoreFloat4x4(&model_matrix, modelMatrix);
		}

	} static_t_old;

	class Statics
	{
	public:
		Statics(AllResources* p_Resources);
		~Statics();

		int current_selected_index;
		int total_static_models;

		bool bRandomness;

		bool bLoadingComplete;

		void SetRandomness(bool _rand) { bRandomness = _rand; }

		void MakePhysics();

		void LoadModels();
		void CreateOne(float x, float y, float z, float height_from_t, float _scale=1.0f, float rx=0.0f, float ry=0.0f, float rz=0.0f);
		void Clear(float x, float z);

		bool LoadBinary(int level);
		bool SaveBinary(int level);
		
		void SetCurrentSelectedindex(int _ind);

		void Initialize(Fire* p_Fire, steam* p_Steam) {
			m_Fire = p_Fire; m_Steam = p_Steam;};

		static_model_t* m_static_model[50];

		MeshMotion** m_Motion;

		void MakeAllLights();
				
		void Reset();

		bool bMakePhysicsWhenLoaded;

		int cur_phy;

		int current_index;
		int current_index2;

		static_t* m_static;
		int max_num;
		int cur_total;

		Flag* m_Flag;

		Fire* m_Fire;
		steam* m_Steam;

		std::vector<part_index> p_index;
		std::vector<part_index> p_index_car;

		SkinnedMeshRenderer m_skinnedMeshRenderer; // for animated trees
		
		Concurrency::task<void> Statics::Update(float timeDelta, float timeTotal);
		void Render(int alpha_mode);
		void Render(int alpha_mode, bool _bdef = false);
		void Statics::RenderDepth(int alpha_mode, int point_plane=-1);
		void RenderPointer();
		void FindCenters();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Resources;
	};

}