#include "pch.h"
#include "Buggy.h"


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;


Buggy::Buggy(AllResources* p_Resources) : CarRig(p_Resources)
{

}

void Buggy::Initialize(Level* pp_Level, int type)
{
	memset(&cvalues, 0, sizeof(car_values));

	switch (type)
	{
	case 0:
	{
		cvalues.car_scale = 1.0f;

		cvalues.body_scale = 0.7f;
		//strcpy(cvalues.body_fname, "buggy.cmo");
		cvalues.body_fname = L"buggy.cmo";
		strcpy_s(cvalues.body_tex1, "carframe");
		strcpy_s(cvalues.body_tex0, "carbody_red");

		cvalues.body_alpha[0] = 0;
		cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.28f;
		//strcpy(cvalues.wheel_fname, "wheel2.cmo");
		cvalues.wheel_fname = L"wheel2.cmo";

		strcpy_s(cvalues.wheel_tex0, "wheel2");
		strcpy_s(cvalues.wheel_nor0, "");

		cvalues.wheel_alpha[0] = 0;

		cvalues.wheel_x_off = 0.6f;
		cvalues.wheel_y_off = -0.09f;
		cvalues.wheel_z_off = 0.8f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 0.03f;
		cvalues.body_mass = 1.2f;

		cvalues.front_friction = 2.0f;
		cvalues.back_friction = 1.8f;

		cvalues.wheel_angular_damping = 0.8f;
		cvalues.wheel_linear_damping = 0.3f;

		cvalues.enable_antistropic_friction = false;

		cvalues.antistropic_friction_x = 1.0f;
		cvalues.antistropic_friction_y = 0.3f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = true;

		cvalues.sus_stiffness = 60.1f;
		cvalues.sus_damping = 0.5f;
		cvalues.sus_travel = 0.8f;

		cvalues.max_speed = 45.0f;
		cvalues.torque = 0.02f;
		cvalues.max_wheel_turn = 0.3f;


		cvalues.body_floor_distance = 0.3f;
		break;
	}
	case 1:
	{
		cvalues.car_scale = 1.2f;

		cvalues.body_scale = 0.7f;
		//strcpy(cvalues.body_fname, "buggy_long");
		cvalues.body_fname = L"buggy_long.cmo";
		strcpy_s(cvalues.body_tex0, "carbody");
		strcpy_s(cvalues.body_tex1, "carframe");

		cvalues.body_alpha[0] = 0;
		cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.45f;
		//strcpy(cvalues.wheel_fname, "wheel");
		cvalues.wheel_fname = L"wheel.cmo";
		strcpy_s(cvalues.wheel_tex0, "yell");
		cvalues.wheel_alpha[0] = 0;

		cvalues.wheel_x_off = 0.3f;
		cvalues.wheel_y_off = -0.16f;
		cvalues.wheel_z_off = 0.9f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 1.0f;
		cvalues.body_mass = 8.0f;

		cvalues.front_friction = 1.2f;
		cvalues.back_friction = 1.0f;

		cvalues.wheel_angular_damping = 0.8f;
		cvalues.wheel_linear_damping = 0.0f;

		cvalues.antistropic_friction_x = 0.5f;
		cvalues.antistropic_friction_y = 0.5f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = false;

		cvalues.body_floor_distance = 1.0f;

		break;
	}
	case 2:
	{
		cvalues.car_scale = 1.5f;

		cvalues.body_scale = 0.7f;
		//strcpy(cvalues.body_fname, "buggy.cmo");
		cvalues.body_fname = L"buggy.cmo";
		strcpy_s(cvalues.body_tex1, "carframe");
		strcpy_s(cvalues.body_tex0, "carbody_blue");

		cvalues.body_alpha[0] = 0;
		cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.45f;
		//strcpy(cvalues.wheel_fname, "wheel2.cmo");
		cvalues.wheel_fname = L"wheel3.cmo";

		strcpy_s(cvalues.wheel_tex0, "woodfloor");
		strcpy_s(cvalues.wheel_tex1, "woodfloor");
		cvalues.wheel_alpha[0] = 0;
		cvalues.wheel_alpha[1] = 0;

		cvalues.wheel_x_off = 0.6f;
		cvalues.wheel_y_off = -0.09f;
		cvalues.wheel_z_off = 0.7f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 0.2f;
		cvalues.body_mass = 0.7f;

		cvalues.front_friction = 2.0f;
		cvalues.back_friction = 2.0f;

		cvalues.wheel_angular_damping = 0.8f;
		cvalues.wheel_linear_damping = 0.3f;

		cvalues.antistropic_friction_x = 1.0f;
		cvalues.antistropic_friction_y = 0.5f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = true;

		cvalues.sus_stiffness = 100.1f;
		cvalues.sus_damping = 0.5f;
		cvalues.sus_travel = 0.8f;

		cvalues.max_speed = 55.0f;
		cvalues.torque = 0.05f;
		cvalues.max_wheel_turn = 0.4f;


		cvalues.body_floor_distance = 1.0f;
		break;
	}
	case 3:
	{
		cvalues.car_scale = 1.0f;

		cvalues.body_scale = 0.7f;
		//strcpy(cvalues.body_fname, "buggy.cmo");
		cvalues.body_fname = L"buggy.cmo";
		strcpy_s(cvalues.body_tex1, "carframe");
		strcpy_s(cvalues.body_tex0, "carbody_blue");

		cvalues.body_alpha[0] = 0;
		cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.3f;
		//strcpy(cvalues.wheel_fname, "wheel2.cmo");
		cvalues.wheel_fname = L"wheel2.cmo";

		strcpy_s(cvalues.wheel_tex0, "wheel2");
		strcpy_s(cvalues.wheel_nor0, "wheel_n");

		cvalues.wheel_alpha[0] = 0;

		cvalues.wheel_x_off = 0.6f;
		cvalues.wheel_y_off = -0.09f;
		cvalues.wheel_z_off = 0.7f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 0.2f;
		cvalues.body_mass = 0.7f;

		cvalues.front_friction = 2.0f;
		cvalues.back_friction = 2.0f;

		cvalues.wheel_angular_damping = 0.8f;
		cvalues.wheel_linear_damping = 0.3f;

		cvalues.antistropic_friction_x = 1.0f;
		cvalues.antistropic_friction_y = 0.5f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = true;

		cvalues.sus_stiffness = 100.1f;
		cvalues.sus_damping = 0.5f;
		cvalues.sus_travel = 0.8f;

		cvalues.max_speed = 35.0f;
		cvalues.torque = 0.03f;
		cvalues.max_wheel_turn = 0.4f;


		cvalues.body_floor_distance = 1.0f;
		break;
	}
	case 4:
	{
		cvalues.car_scale = 1.5f;

		cvalues.body_scale = 0.8f;
		//strcpy(cvalues.body_fname, "datsunbody");
		strcpy_s(cvalues.body_tex0, "datsuntexture");


		cvalues.body_alpha[0] = 0;
		//cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.85f;
		//strcpy(cvalues.wheel_fname, "datsunwheel");
		strcpy_s(cvalues.wheel_tex0, "datsuntexture");
		cvalues.wheel_alpha[0] = 0;



		cvalues.wheel_x_off = 0.50f;
		cvalues.wheel_y_off = -0.1f;
		cvalues.wheel_z_off = 0.82f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 1.0f;
		cvalues.body_mass = 2.0f;

		cvalues.front_friction = 2.2f;
		cvalues.back_friction = 2.0f;

		cvalues.wheel_angular_damping = 0.0f;
		cvalues.wheel_linear_damping = 0.0f;

		cvalues.antistropic_friction_x = 1.0f;
		cvalues.antistropic_friction_y = 0.3f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = true;

		cvalues.sus_stiffness = 200.1f;
		cvalues.sus_damping = 0.5f;
		cvalues.sus_travel = 1.0f;

		cvalues.max_speed = 40.0f;
		cvalues.torque = 0.2f;
		cvalues.max_wheel_turn = 0.3f;

		cvalues.body_floor_distance = 1.0f;
		break;
	}
	case 5:
	{
		cvalues.car_scale = 1.0f;

		cvalues.body_scale = 0.7f;
		//strcpy(cvalues.body_fname, "buggy");
		strcpy_s(cvalues.body_tex1, "carframe");
		strcpy_s(cvalues.body_tex0, "carbody");

		cvalues.body_alpha[0] = 0;
		cvalues.body_alpha[1] = 0;

		cvalues.wheel_scale = 0.55f;
		//strcpy(cvalues.wheel_fname, "wheel");
		strcpy_s(cvalues.wheel_tex0, "wheel2");
		cvalues.wheel_alpha[0] = 0;

		cvalues.wheel_x_off = 0.3f;
		cvalues.wheel_y_off = -0.09f;
		cvalues.wheel_z_off = 0.7f;

		cvalues.wheel_rig_scale_to_model = 1.0f;

		cvalues.wheel_mass = 0.3f;
		cvalues.body_mass = 1.0f;

		cvalues.front_friction = 1.5f;
		cvalues.back_friction = 2.0f;

		cvalues.wheel_angular_damping = 0.8f;
		cvalues.wheel_linear_damping = 0.3f;

		cvalues.antistropic_friction_x = 0.3f;
		cvalues.antistropic_friction_y = 0.3f;
		cvalues.antistropic_friction_z = 1.0f;

		cvalues.bPoint2Point = true;

		cvalues.sus_stiffness = 100.1f;
		cvalues.sus_damping = 0.5f;
		cvalues.sus_travel = 0.8f;

		cvalues.max_speed = 40.0f;
		cvalues.torque = 0.03f;
		cvalues.max_wheel_turn = 0.4f;

		cvalues.body_floor_distance = 1.0f;
		break;
	}
	}


	CarRig::Initialize(pp_Level);
}