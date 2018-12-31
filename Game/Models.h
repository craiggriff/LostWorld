#pragma once

#include "modelloader.h"

namespace Game
{
	typedef struct
	{
		ModelLoader* m_Model;
		char filename[30];

	} models_t;

	

	class Models
	{
	public:
		Models(std::shared_ptr<DX::DeviceResources> pm_deviceResources);

		void SetRenderBuffers(char* filename);
		void DrawIndexed(char* filename);

		ModelLoader* LoadModel(char* filename, float scale, int optimize = 0, bool _bUpdateable = false, int _updateLevel = 0); // returns a pointer if already loaded
		ModelLoader* LoadModelFBX(char *filename, float scale, int optimize = 0, bool _bUpdateable = false, int _updateLevel = 0);

		ModelLoader* Models::GetLastAddModelPointer();

		models_t m_Models[50];


	private:
		int current;
		int last_add_index;
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}