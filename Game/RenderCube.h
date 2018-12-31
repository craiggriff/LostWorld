#pragma once
////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
namespace Game
{
	class  RenderCube
	{
	public:
		RenderCube();
		~RenderCube();

		int tex_width, tex_height;

		bool Initialize(std::shared_ptr<DX::DeviceResources> pm_deviceResources, int size_width, int size_height, DXGI_FORMAT tex_format);
		void Shutdown();

		void ClearTargets();

		DirectX::XMMATRIX RenderCubeSide(int _side, DirectX::XMFLOAT4 target_col=DirectX::XMFLOAT4(1.0f,0.0f,0.0f,1.0f));

		void SetRenderPosition(float x, float y, float z) {
			render_position.x = x;
			render_position.y = y;
			render_position.z = z;
		}



		void RenderCube::CreateSpotProjection(
			_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
			_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
			_In_ float nearPlane,           // depth to map to 0
			_In_ float farPlane             // depth to map to 1
		);

		DirectX::XMFLOAT4X4 spot_projection;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();
		DirectX::XMFLOAT3 render_position;



		

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Members for the point light depth shadows map. 
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_pointshadowMapRenderTargetView[6];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_pointshadowMapShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_pointshadowMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pointshadowMapTexture;

		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_sampler;                    // cube texture sampler
	private:
	};
}