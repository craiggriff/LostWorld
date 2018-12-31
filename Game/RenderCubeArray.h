#pragma once
////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
namespace Game
{
	class  RenderCubeArray
	{
	public:
		RenderCubeArray();
		~RenderCubeArray();

		int tex_width, tex_height, total_cubes;

		bool Initialize(std::shared_ptr<DX::DeviceResources> pm_deviceResources, int size_width, int size_height, int num_cubes, DXGI_FORMAT tex_format,int size_width_to=0,int size_height_to=0);
		void Shutdown();

		void ClearTargets();

		DirectX::XMMATRIX RenderCubeSide(int _cube_num, int _side);

		void SetRenderPosition(int indx , float x, float y, float z) {
			render_position[indx].x = x;
			render_position[indx].y = y;
			render_position[indx].z = z;
		}

		void CreateSpotProjection(
			_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
			_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
			_In_ float nearPlane,           // depth to map to 0
			_In_ float farPlane             // depth to map to 1
		);

		DirectX::XMFLOAT4X4 spot_projection;

		//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();
		//aID3D11ShaderResourceView* RenderCubeArray::GetShaderResourceView();

		DirectX::XMFLOAT3* render_position;

	



		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		/*
		// Members for the point light depth shadows map. 
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>*     m_pointshadowMapRenderTargetView;
		//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_pointshadowMapShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_pointshadowMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapTexture;
		*/
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>*     m_renderTargetView;
		//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_pointshadowMapShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_renderTexture;

		ID3D11ShaderResourceView* m_shaderResourceView[50];

		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_sampler;                    // cube texture sampler
	private:
	};
}