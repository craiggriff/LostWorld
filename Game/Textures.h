#pragma once
#include "../DirectXTK/Inc/DDSTextureLoader.h"



namespace Game
{
	typedef struct
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture;
		char filename[30];

	} texture_t;



	class Textures
	{
	public:
		Textures(std::shared_ptr<DX::DeviceResources> pm_deviceResources, bool _bHighRez);
		~Textures();



		int current;
		bool bHighRez;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(char* filename); // returns a pointer if already loaded

		texture_t textures[100];

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};
}
