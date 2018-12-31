#include "pch.h"
#include "Models.h"

using namespace Game;

using namespace std;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;



Models::Models(std::shared_ptr<DX::DeviceResources> pm_deviceResources)
{
	m_deviceResources = pm_deviceResources;
	current = 0;
}

void Models::SetRenderBuffers(char* filename)
{
	int i;
	for (i = 0; i < current; i++)
	{
		if (!strcmp(m_Models[i].filename, filename))
		{
			m_Models[i].m_Model->SetRenderBuffers();
		}
	}
}

ModelLoader* Models::GetLastAddModelPointer()
{
	return m_Models[last_add_index].m_Model;

}



void Models::DrawIndexed(char* filename)
{
	int i;
	for (i = 0; i < current; i++)
	{
		if (!strcmp(m_Models[i].filename, filename))
		{
			m_Models[i].m_Model->DrawIndexed();
		}
	}
}


ModelLoader* Models::LoadModel(char *filename, float scale, int optimize, bool _bUpdateable, int _updateLevel)
{
	int i;
	bool bFound;
	char full_filename[60];

	for (i = 0; i < current; i++)
	{
		if (!strcmp(m_Models[i].filename, filename))
		{
			bFound = true;
			return m_Models[i].m_Model;
		}
	}

	sprintf(full_filename, "Assets/Models/%s.obj", filename);
	m_Models[current].m_Model = new ModelLoader(m_deviceResources, _bUpdateable, _updateLevel);
	m_Models[current].m_Model->Load(full_filename, scale, optimize);

	strcpy(m_Models[current].filename, filename);

	last_add_index = current;

	current++;

	return m_Models[current - 1].m_Model;

}



ModelLoader* Models::LoadModelFBX(char *filename, float scale, int optimize, bool _bUpdateable, int _updateLevel)
{
	int i;
	bool bFound;
	char full_filename[60];

	for (i = 0; i < current; i++)
	{
		if (!strcmp(m_Models[i].filename, filename))
		{
			bFound = true;
			return m_Models[i].m_Model;
		}
	}

	sprintf(full_filename, "%s", filename);
	m_Models[current].m_Model = new ModelLoader(m_deviceResources, _bUpdateable, _updateLevel);
	m_Models[current].m_Model->LoadFBX(full_filename, scale);

	strcpy(m_Models[current].filename, filename);

	last_add_index = current;

	current++;

	return m_Models[current - 1].m_Model;

}
