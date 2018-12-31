////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>


////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
namespace Game
{
	class  RenderTextureClass
	{
	public:
		RenderTextureClass();
		RenderTextureClass(const RenderTextureClass&);
		~RenderTextureClass();

		int tex_width, tex_height;

		bool Initialize(ID3D11Device*, int, int, DXGI_FORMAT tex_format, int _mips=1);
		void Shutdown();

		void SetRenderTarget(ID3D11DeviceContext* deviceContext);
		void SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* stencil);
		void RestoreRenderTarget(ID3D11DeviceContext* deviceContext);

		void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView();

		//Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_environmentMapDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_depthStencilTexture;
	private:
		D3D11_VIEWPORT previousViewport;
		D3D11_VIEWPORT environmentMapViewport;

		ID3D11Texture2D* m_renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;



		//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_environmentMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> previousRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> previousDepthStencilView;
	};
}
#endif