#include "pch.h"
#include "Bert.h"


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;


Bert::Bert(AllResources* p_Resources, Level* pp_Level) : AnimChar(p_Resources, pp_Level)
{



	
}



void Bert::Initialize(int char_num)
{
	switch (char_num)
	{
	case 0:
	{
		cvalues.model_fname = L"dark_templar_knightc.cmo";
		strcpy_s(cvalues.texture_fname, "knight_d");
		strcpy_s(cvalues.normal_fname, "knight_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.124f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.

	}
	break;
	case 1:
	{
		cvalues.model_fname = L"mikey.cmo";
		strcpy_s(cvalues.texture_fname, "mickey_mouse_d");
		strcpy_s(cvalues.normal_fname, "mickey_mouse_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.

	}
	break;
	case 2:
	{
		cvalues.model_fname = L"animsanta.cmo";
		strcpy_s(cvalues.texture_fname, "mickey_mouse_d");
		strcpy_s(cvalues.normal_fname, "mickey_mouse_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.

	}
	break;
	case 3:
	{
		cvalues.model_fname = L"mario.cmo";
		strcpy_s(cvalues.texture_fname, "mickey_mouse_d");
		strcpy_s(cvalues.normal_fname, "mickey_mouse_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.

	}
	break;
	}


	AnimChar::Initialize();
}