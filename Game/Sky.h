#pragma once
#include <vector>

#include "../DirectXTK/Inc/DDSTextureLoader.h"


#include "Camera.h"
#include "RenderCube.h"


#define NumberOfFaces 6

namespace Game
{

	class Sky
	{
	public:
		Sky(AllResources* pm_Resources);
		~Sky(void);

		float angle;

		float x, y, z;

		float pos_y;

		char last_filename[40];

		ModelViewProjectionConstantBuffer	m_constantBufferData;

		void SetBuffers(Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer, UINT indexCount);

		void LoadLevel(int level);

		int m_indexCount;
		char skybox[30];

		void Initialize(float _size, float _ypos);

		void Render();
		void RenderCenter();

		void ReleaseTexture();

		void LoadSkybox(char* f_name);

		DirectX::XMFLOAT4X4 m_Matrix;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		bool	m_loadingComplete;

		RenderCube* m_SkyCubeMap;
		RenderCube* m_SkyPlaneCubeMap;

		bool bEnvironmentCreated;

		// Members for the environment map. 
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_environmentMapRenderTargetView[NumberOfFaces];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_environmentMapShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_environmentMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_environmentMapDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_environmentMapTexture;
		int m_environmentMapWidth;         // The width of the texture on each cubemap face.
		int m_environmentMapHeight;        // The height of the texture on each cubemap face.

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Resources;




	};

}