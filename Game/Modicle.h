#pragma once


namespace Game
{
	class Modicle
	{
	public:
		Modicle(std::shared_ptr<DX::DeviceResources> pm_deviceResources, ModelLoader* pp_ModelLoader, int maximum, int _sort = 0);
		~Modicle(void);

		int m_maxParticles;
		int total_count;

		bool bSorting;

		DirectX::XMVECTOR* tcenter;  // triangle center
		part_index* p_index;

		void SetCamera(float x, float y, float z);
		DirectX::XMFLOAT3 cam_pos;

		

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_point_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_point_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_point_inputLayout;

		ModelLoader* p_ModelLoader;

		VertexData* m_vertices;
		unsigned long* m_indices;

		VertexData* m_model_vertices; // local copy of single object
		unsigned long* m_model_indices;


		int m_vertexCount, m_indexCount, m_modelvertexCount, m_modelindexCount;



		ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

		void Reset();
		void AddOne(DirectX::XMFLOAT4X4* matr, DirectX::XMFLOAT4* _cols = nullptr);

		bool InitializeBuffers();
		bool UpdateVertecies(ID3D11DeviceContext* deviceContext);
		void Render(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> deviceContext, Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer, ModelViewProjectionConstantBuffer* constantBufferData);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};

}
