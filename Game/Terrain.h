#pragma once
#include <fstream>
#include <string>
#include <vector>
//#include "DirectXHelper.h"
//#include "VertexData.h"
//#include "MVPConstantBuffer.h"

#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "../Bullet/src/btBulletDynamicsCommon.h"


//#include "DDSTextureLoader.h"
#include "Physics.h"
//#include "Object3D.h"

using namespace std;
//using namespace DirectX;
//using namespace DX;
//using namespace Microsoft::WRL;

namespace Game
{
	class SimpleMotion : btMotionState
	{
	public:
		SimpleMotion();
		~SimpleMotion();

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);

		btTransform m_initialTransform;
	};



	class Terrain
	{
	public:
		Terrain(AllResources* p_Resources, int _bUpdateable = 0);
		~Terrain(void);

		// Constant buffer
		btTransform m_initialTransform;
		ModelViewProjectionConstantBuffer	m_constantBufferData;

		AllResources* m_Resources;

		bool bPhysicsObject;

		int xcoords, ycoords;

		int xblockpos, yblockpos;

		bool bLoaded;

		bool bUpdateable;

		bool b_in_scene;

		float cam_check_point;
		float cam_check_point_full;


		float pos_x, pos_y, pos_z;


		int start_id, end_id;

		bool LoadFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols,int xnum, int ynum, float scale, int xplane, float** _steepness, float*** _tex_blend);
		bool UpdateFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols, float** _steepness, float*** _tex_blend);

		void CreateBuffers();
		void RemovePhysics();
		
		bool CreateUpdatebleVertexBuffer();
		bool UpdateUpdatebleVertexBuffer();
		
		float** steepness;
		float*** tex_blend;

		float** height_map;
		DirectX::XMFLOAT3** normals;
		DirectX::XMFLOAT4** cols;

		DirectX::XMFLOAT3** tangent;
		DirectX::XMFLOAT3** binorm;

		float* height_fields;


		float average_height;
		float furthest_point;

		SimpleMotion* m_box;

		void InitTerrain(int xbp, int ybp);

		void CalculateCenterHeight();

		float zpos;
		float xpos;

		btRigidBody* m_rigidbody;
		btCollisionShape* m_collisionShape;

		float* cam_z;

		HeightMapInfo* hm_info;

		part_index* p_index;

		float m_scale;

		VertexType* m_phy_vertices;
		int no_phy_verticies;
		int count_no_phy_verticies;
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return m_vertexBuffer; }
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return m_indexBuffer; }
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBufferSimple() { return m_indexBuffer_simple; }

		void SetVertexBuffer();
		void SetIndexBuffer();
		void SetIndexBufferSimple();
		void ClearMemory();

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);


		void SetPosition(float x, float y, float z);

		vector<GroundVertexData> m_vertices;
		vector<unsigned short> m_indices;
		vector<unsigned short> m_indices_simple;

		void AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz);
		void AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a, float b1, float bl1, float bl2, float bl3, float bl4, float bl5, float bl6 );
		void Render(int complexity);

		void GetMatrix(DirectX::XMFLOAT4X4* matr);

		void UpdateProjectionMatrix(DirectX::XMMATRIX *projectionMatrix);
		void Update(DirectX::XMMATRIX *viewMatrix, float timeTotal);

		void CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal);
		void CalculateTerrainVectors();

		void MakePhysicsConvexTriangleTerrain();

		//void LoadTexture(Microsoft::WRL::ComPtr<ID3D11Device1> pm_d3dDevice, wchar_t* texture_filename);


		int m_verticesCount;
		int m_indicesCount;
		int m_indicesCount_simple;

		Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;

		ID3D11ShaderResourceView* m_Texture;

		// Vertex and index buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer_simple;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}

