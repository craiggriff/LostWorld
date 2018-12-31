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

	class FlagNode
	{
	public:
		FlagNode(AllResources* p_Resources, int _bUpdateable = 0);
		~FlagNode(void);

		// Constant buffer
		btTransform m_initialTransform;
		ModelViewProjectionConstantBuffer	m_constantBufferData;

		AllResources* m_Resources;

		bool bPhysicsObject;

		int xcoords, ycoords;

		int xblockpos, yblockpos;

		bool bLoaded;

		bool bUpdateable;

		int start_id, end_id;

		bool LoadFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols,int xnum, int ynum, float scale, int xplane);
		bool UpdateFromHeightMap(float** whole_height_map, norm_t** whole_normals, DirectX::XMFLOAT4** p_cols);

		void CreateBuffers();
		void UpdateBuffers();
		
		
		bool CreateUpdatebleVertexBuffer();
		bool UpdateUpdatebleVertexBuffer();
		
		float** height_map;
		DirectX::XMFLOAT3** normals;
		DirectX::XMFLOAT4** cols;

		float* height_fields;

		SimpleMotion* m_box;

		void InitFlagNode(int xbp, int ybp, int _total_x, int _total_y);

		int total_x, total_y;

		float zpos;
		float xpos;

		btRigidBody* m_rigidbody;

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

		vector<VertexData> m_vertices;
		vector<unsigned short> m_indices;
		vector<unsigned short> m_indices_simple;

		void AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz);
		void AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a);
		void Render(int complexity);

		void GetMatrix(DirectX::XMFLOAT4X4* matr);

		void UpdateProjectionMatrix(DirectX::XMMATRIX *projectionMatrix);
		void Update(DirectX::XMMATRIX *viewMatrix, float timeTotal);



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

