////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _RENDERTEXTUREARRAY_H_
#define _RENDERTEXTUREARRAY_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>


////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
namespace Game
{
	class  RenderTextureArray
	{
	public:
		RenderTextureArray();
		RenderTextureArray(const RenderTextureArray&);
		~RenderTextureArray();

		int tex_width, tex_height;

		int tex_num;

		bool Initialize(ID3D11Device*, int, int, int num_textures, DXGI_FORMAT tex_format);
		void Shutdown();

		void SetRenderTarget(int _num, ID3D11DeviceContext* deviceContext);
		void SetRenderTarget(int _num, ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* stencil);

		void ClearRenderTarget(int _num, ID3D11DeviceContext* deviceContext,float red, float green, float blue, float alpha);
		ID3D11ShaderResourceView* GetShaderResourceView();

		ID3D11ShaderResourceView* m_shaderResourceView[50];
	private:
		D3D11_VIEWPORT previousViewport;
		D3D11_VIEWPORT environmentMapViewport;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>* m_renderTargetView;


		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_environmentMapDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_environmentMapDepthStencilTexture;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> previousRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> previousDepthStencilView;
	};
}
#endif