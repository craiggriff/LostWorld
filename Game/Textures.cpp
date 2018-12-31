#include "pch.h"
#include "Textures.h"


using namespace Game;
using namespace DirectX;
using namespace DX;

Textures::Textures(std::shared_ptr<DX::DeviceResources> pm_deviceResources, bool _bHighRez)
{
	int i;
	current = 0;
	bHighRez = _bHighRez;

	m_deviceResources = pm_deviceResources;
	for (i = 0; i < 50; i++)
	{
		textures[i].m_Texture = nullptr;
	}

}

Textures::~Textures()
{
	// Release all textures

}

inline bool exists_test1(char* name) {
	FILE *file;
	fopen_s(&file, name, "r");

	if (file) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Textures::LoadTexture(char* filename)
{
	int i;
	bool bFound;

	char full_filename[60];
	wchar_t w_full_filename[60];

	for (i = 0; i < current; i++)
	{
		if (!strcmp(textures[i].filename, filename))
		{
			bFound = true;
			return textures[i].m_Texture;
		}
	}

	if (bHighRez == false)
	{
		sprintf_s(full_filename, "Assets/Textures/Low/%s.dds", filename);
	}
	else
	{
		sprintf_s(full_filename, "Assets/Textures/%s.dds", filename);
		if (exists_test1(full_filename) == false)
		{
			sprintf_s(full_filename, "%s.dds", filename);
		}
	}
	size_t bytes_conv;
	mbstowcs_s(&bytes_conv, w_full_filename, full_filename, 60);

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), w_full_filename, nullptr, &textures[current].m_Texture, MAXSIZE_T);

	//textures[current].m_Texture.CopyTo()


	strcpy_s(textures[current].filename, filename);

	current++;

	return textures[current - 1].m_Texture;
}
