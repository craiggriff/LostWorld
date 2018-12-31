#pragma once

//#include "Coin.h"


#include "FlagNode.h"
#include "Camera.h"



/*
#pragma pack(2) // Add this

typedef struct
{
unsigned short bfType;
unsigned int   bfSize;
unsigned short bfReserved1;
unsigned short bfReserved2;
unsigned int   bfOffBits;
} BITMAPFILEHEADER;

#pragma pack() // and this
*/


namespace Game
{

	class Flag
	{
	public:
		Flag(AllResources* p_Resources);
		~Flag(void);



		int total_planes;

		int left_wall_id;
		int right_wall_id;
		int front_wall_id;
		int back_wall_id;

		int current_index;

		float scale_height;


		int map_x_size, map_y_size;

		int total_platforms;

		float left_right_walls;
		float front_back_walls;

		float back_position;

		float* cam_z;

		float** height_map;
		norm_t** normals;
		DirectX::XMFLOAT4** colours;


		float highest_point;
		float lowest_point;

		int total_x_points;
		int total_y_points;

		char ground1[30];
		char ground2[30];
		float noise_scale;

		float local_terrain_scale;

		int x_planes;// = ((map_x_size) / 9);
		int y_planes;// = ((map_y_size) / 9);

		int lev_num;

		int clampB(int pX, int pMax);

		void CalculateNormals();


		btMatrix3x3 rot_mat;
		btMatrix3x3 flag_rot_mat;

		


		void InitializeGeneratedTerrain(int Flag, float height_scale, float noise_z);
		void UpdateGeneratedTerrain(int Flag, float noise_z, DirectX::XMFLOAT4X4 _rot_mat);


		void InitializeTerrainVariables(int x_nodes, int y_nodes, float terrain_scale);

		void Update(DirectX::XMMATRIX *viewMatrix, float timeTotal);

		void Render(float x, float y, float z);

		float GetTerrainHeight(float xPos, float zPos);

		void UpdateFlagNodes();

		float ang;

		btVector3 Flag::GetNormal(float xPos, float zPos);

		float m_height_scale;

		void ClearMemory();

		FlagNode* m_FlagNode[500];

		int current_location;
		bool bLoadedShadows;

		HeightMapInfo m_hminfo;

		Physics* m_phys;
		float m_terrain_scale;

		int fin_state;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture;



	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Resources;
	};

}