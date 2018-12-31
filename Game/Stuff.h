#pragma once


using namespace std;


#include "flag.h"


namespace Game
{



	typedef struct
	{
		//FBXModelLoader* model;
		std::vector<Mesh*> m_meshModels;
		DirectX::XMFLOAT3 mrf;
		int physical_shape; // 0 = box , 1 = cyl, 2 = sphere
		int collision_range;// 0 = forever, 1 = close
		int group;
		int mask;
	} stuff_model_t;

	typedef struct
	{
	public:
		int bActive;
		float scale;

		float height_from_terrain; // used for terrain changes
		DirectX::XMFLOAT4X4 model_matrix;
		int model_index;

		ObjInfo info;
		int _padd;
		DirectX::XMFLOAT4 colour;

		DirectX::XMFLOAT4 spare[50];

	} stuff_t;

	class Stuff
	{
	public:
		Stuff(AllResources* p_Resources);
		~Stuff();

		int current_selected_index;
		int total_stuff_models;

		void LoadStuff(bool _setPhysical);
		void CreateOne(float x, float y, float z, float height_from_t, float _scale = 1.0f, float rx = 0.0f, float ry = 0.0f, float rz = 0.0f, bool _FromLastHeight=false);
		void Clear(float x, float z);

		bool LoadBinary(int level);
		bool SaveBinary(int level);

		void SetCurrentSelectedindex(int _ind);

		void SetPhysical(bool _bIsPhysical);

		void Reset();

		stuff_model_t* m_stuff_model[10];

		stuff_t* m_stuff;

		bool bLoadingComplete;

		MeshMotion** m_Motion;

		int cur_phy;



		int max_num;
		int cur_total;
		
		
		float found_y;
		int index_found;

		bool bPhysical;

		int current_index;

		float stack_pointer_y;

		std::vector<part_index> p_index;
		std::vector<part_index> p_index_car;

		Concurrency::task<void> Update();
		void Render(int alpha_mode);
		void RenderDepth(int alpha_mode, int point_plane = -1);

		void RenderPointer(DirectX::XMFLOAT4X4* t_matrix, bool _FromLastHeight=false);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Resources;
	};

}