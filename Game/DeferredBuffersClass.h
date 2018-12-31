////////////////////////////////////////////////////////////////////////////////
// Filename: deferredbuffersclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _DEFERREDBUFFERSCLASS_H_
#define _DEFERREDBUFFERSCLASS_H_


/////////////
// DEFINES //
/////////////
#define DEFERRED_BUFFER_COUNT 4


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>

namespace Game
{

	////////////////////////////////////////////////////////////////////////////////
	// Class name: DeferredBuffersClass
	////////////////////////////////////////////////////////////////////////////////
	class DeferredBuffersClass
	{
	public:
		DeferredBuffersClass();
		DeferredBuffersClass(const DeferredBuffersClass&);
		~DeferredBuffersClass();

		bool Initialize(ID3D11Device*, int, int, float, float);
		void Shutdown();

		void SetRenderTargets(ID3D11DeviceContext*);
		void SetRenderTargets(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* stencil);
		void ClearRenderTargets(ID3D11DeviceContext*, float, float, float, float);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView(int);

	private:
		int m_textureWidth, m_textureHeight;
		ID3D11Texture2D* m_renderTargetTextureArray[DEFERRED_BUFFER_COUNT];
		ID3D11RenderTargetView* m_renderTargetViewArray[DEFERRED_BUFFER_COUNT];
		ID3D11ShaderResourceView* m_shaderResourceViewArray[DEFERRED_BUFFER_COUNT];
		ID3D11Texture2D* m_depthStencilBuffer;
		ID3D11DepthStencilView* m_depthStencilView;
		D3D11_VIEWPORT m_viewport;
	};
}

#endif